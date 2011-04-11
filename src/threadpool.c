/*
 * See Licensing and Copyright notice in threadpool.h
 */
/*
 * @brief A simple threadpool implementation using a single queue.
 *
 * The queue is inspired by this paper (look for the queue with two locks):
 * http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.109.5602&rep=rep1&type=pdf
 */


#include "threadpool.h"

#include "SDL.h"
#include "SDL_thread.h"

#include <stdlib.h>

#include "log.h"


#define THREADPOOL_TIMEOUT (5 * 100) /* The time a worker thread waits */
#define THREADSIG_STOP (1) /* The signal to stop a worker thread */
#define THREADSIG_RUN (0) /* The signal to indicate the worker thread is running */


#if SDL_VERSION_ATLEAST(1,3,0)
const int MAXTHREADS = SDL_GetCPUCount()+1;
#else
const int MAXTHREADS = 8;
#endif


struct Node_ {
   void *data;          /* The element in the list */
   struct Node_ *next;  /* The next node in the list */
} Node_;
typedef struct Node_ *Node;

struct ThreadQueue_ {
   Node first;          /* The first node */
   Node last;           /* The second node */
   /* A semaphore to ensure reads only happen when the queue is not empty */
   SDL_sem *semaphore;  
   SDL_mutex *t_lock;   /* Tail lock. Lock when reading/updating tail */
   SDL_mutex *h_lock;   /* Same as tail lock, except it's head lock */
} ThreadQueue_;

typedef struct ThreadQueue_data_ {
   int (*function)(void *);     /* The function to be called */
   void *data;                  /* And its arguments */
} ThreadQueue_data_;
typedef ThreadQueue_data_ *ThreadQueue_data;

typedef struct ThreadData_ {
   int (*function)(void *); /* The function to be called */
   void *data; /* Arguments to the above function */
   int signal; /* Signals to the thread */
   SDL_sem *semaphore; /* The semaphore to signal new jobs or new signal in the
                          'signal' variable */
   ThreadQueue idle; /* The queue with idle threads */
   ThreadQueue stopped; /* The queue with stopped threads */
} ThreadData_;

typedef struct vpoolThreadData_ {
   SDL_cond *cond; /* Condition variable for signalling all jobs in the vpool
                      are done */
   SDL_mutex *mutex; /* The mutex to use with the above condition variable */
   int *count; /* Variable to count number of finished jobs in the vpool */
   ThreadQueue_data node; /* The job to be done */
} vpoolThreadData_;
typedef vpoolThreadData_ *vpoolThreadData;

/* The global threadpool queue */
static ThreadQueue global_queue = NULL;


/*
 * Prototypes.
 */
static ThreadQueue tq_create (void);
static void tq_enqueue( ThreadQueue q, void *data );
static void* tq_dequeue( ThreadQueue q );
static void tq_destroy( ThreadQueue q );
static int threadpool_worker( void *data );
static int threadpool_handler( void *data );
static int vpool_worker( void *data );

/** 
 * @brief Creates a concurrent queue.
 * @return The ThreadQueue.
 */
static ThreadQueue tq_create (void)
{
   ThreadQueue q;
   Node n;

   q = calloc( 1, sizeof(ThreadQueue_) );

   /* Allocate and insert the dummy node */
   n = calloc( 1, sizeof(Node_) );
   n->next = NULL;
   q->first = n;
   q->last = n;

   q->t_lock = SDL_CreateMutex();
   q->h_lock = SDL_CreateMutex();
   q->semaphore = SDL_CreateSemaphore( 0 );

   return q;
}

/**
 * @brief Enqueue data to the ThreadQueue q.
 *
 * @param q The queue to be inserted into.
 * @param data The element to be stored in the queue.
 */
static void tq_enqueue( ThreadQueue q, void *data )
{
   Node n;

   n = calloc( 1, sizeof(Node_) );
   n->data = data;
   n->next = NULL;

   /* Lock */
   SDL_mutexP(q->t_lock);

   q->last->next = n;
   q->last = n;

   /* Signal and unlock. This wil break if someone tries to enqueue 2^32+1
    * elements or something. */
   SDL_SemPost(q->semaphore);
   SDL_mutexV(q->t_lock);
}

/**
 * @brief Dequeue from the ThreadQueue q.
 * @attention The callee should ALWAYS have called SDL_SemWait() on the semaphore.
 *
 * @param q The queue to dequeue from.
 *
 * @return A void pointer to the element from the queue.
 */
static void* tq_dequeue( ThreadQueue q )
{
   void *d;
   Node newhead, node;
   
   /* Lock */
   SDL_mutexP(q->h_lock);

   node = q->first;
   newhead = node->next;

   if (newhead == NULL) {
      WARN("Tried to dequeue while the queue was empty!");
      /* Unlock and return NULL */
      SDL_mutexV(q->h_lock);
      return NULL;
   }
   /* Remember the value and assign newhead as the new dummy element. */
   d = newhead->data;
   q->first = newhead;
   
   /* Unlock */
   SDL_mutexV(q->h_lock);

   free(node);
   return d;
}

/** 
 * @brief Destroy and free a ThreadQueue.
 * Frees all elements too.
 *
 * @param q The ThreadQueue to free.
 * This does not try to lock or anything */
static void tq_destroy( ThreadQueue q )
{
   SDL_DestroySemaphore(q->semaphore);
   SDL_DestroyMutex(q->h_lock);
   SDL_DestroyMutex(q->t_lock);

   /* Iterate through the list and free the nodes */
   while(q->first->next != NULL) {
      free( tq_dequeue(q) );
   }
   
   free( q->first );
   free(q);
}


/**
 * @brief Enqueues a new job for the threadpool. 
 * @warning Do NOT enqueue a job that has to wait for another job to be done as
 * this could lead to a deadlock. 
 *
 * @param function The function (job) to be called (executed).
 * @param data The arguments for the function.
 *
 * @return Returns 0 on success and -2 if there was no threadpool. */
int threadpool_newJob(int (*function)(void *), void *data)
{
   ThreadQueue_data node;

   if (global_queue == NULL) {
      WARN("Threadpool has not been initialized yet!");
      return -2;
   }
   
   node = calloc( 1, sizeof(ThreadQueue_data_) );
   node->data = data;
   node->function = function;

   tq_enqueue( global_queue, node );
   
   return 0;
}

/**
 * @brief The worker function for the threadpool.
 *
 * It waits for a signal from the handler. If it receives THREADSIG_STOP it
 * means the worker thread should stop. Else it dequeues a job from the
 * global_queue and executes it.
 *
 * @param data A pointer to the ThreadData struct used for a lot of stuff.
 */
static int threadpool_worker( void *data )
{
   ThreadData_ *work;
   
   work = (ThreadData_ *) data;

   /* Work loop */
   while (1) {
      /* Wait for new signal */
      while (SDL_SemWait( work->semaphore ) == -1) {
          /* Putting this in a while-loop is probably a really bad idea, but I
           * don't have any better ideas. */
          WARN("L%d: SDL_SemWait failed! Error: %s", __LINE__, SDL_GetError());
      }
      /* Break if received signal to stop */
      if ( work->signal == THREADSIG_STOP ) {
         break;
      }

      /* Do work :-) */
      (*work->function)( work->data );
      
      /* Enqueue itself in the idle worker threads queue */
      tq_enqueue( work->idle, work );
   }
   /* Enqueue itself in the stopped worker threads queue when stopped */
   tq_enqueue( work->stopped, work );

   return 0;
}

/**
 * @brief Handles assigning jobs to the workers and killing them if necessary.
 * Stopping the threeadpool_handler is not yet implemented.
 * 
 * @param data Not used. SDL threading requires functions to take a void
 * pointer as argument.
 *
 * @return Not really implemented yet.
 */
static int threadpool_handler( void *data )
{
   (void) data;
   int i, nrunning;
   ThreadData_ *threadargs, *threadarg;
   /* Queues for idle workers and stopped workers */
   ThreadQueue idle, stopped;
   ThreadQueue_data node;
   
   idle = tq_create();
   stopped = tq_create();

   /* Allocate threadargs to communicate with workers */
   threadargs = calloc( MAXTHREADS, sizeof(ThreadData_) );

   /* Initialize threadargs */
   for (i=0; i<MAXTHREADS; i++) {
      threadargs[i].function = NULL;
      threadargs[i].data = NULL;
      threadargs[i].semaphore = SDL_CreateSemaphore( 0 );
      threadargs[i].idle = idle;
      threadargs[i].stopped = stopped;
      threadargs[i].signal = THREADSIG_RUN;
      /* 'Workers' that do not have a thread are considered stopped */
      tq_enqueue(stopped, &threadargs[i]); 
   }

   /* Set the number of running threads to 0 */
   nrunning = 0;

   /**
    * The main loop.
    * TODO: Make a nice description of what goes on.
    */
   while (1) {
      /* We only have to do this if there are any workers */
      if (nrunning > 0) {
         /* Try wait for a new job */
         if (SDL_SemWaitTimeout( global_queue->semaphore, THREADPOOL_TIMEOUT ) != 0) {
            /* There weren't any new jobs so we'll start killing threads ;) */
            if ( SDL_SemTryWait(idle->semaphore) == 0 ) {
               threadarg = tq_dequeue( idle );
               /* Set signal to stop worker thread */
               threadarg->signal = THREADSIG_STOP;
               /* Signal thread and decrement running threads counter */
               SDL_SemPost( threadarg->semaphore );
               nrunning -= 1;
            }
            /* We want to start waiting for jobs again */
            continue;
         }
      } 
      else {
         /* Wait for a new job */
         if (SDL_SemWait( global_queue->semaphore ) == -1) {
             WARN("L%d: SDL_SemWait failed! Error: %s", __LINE__, SDL_GetError());
             continue;
         }
      }
      /* Get a new job from the queue */
      node = tq_dequeue( global_queue );

      /* Idle thread available */
      if( SDL_SemTryWait(idle->semaphore) == 0) {
         /* Assign arguments for the thread */
         threadarg = tq_dequeue( idle );
         threadarg->function = node->function;
         threadarg->data = node->data;
         /* Signal the thread that there's a new job */
         SDL_SemPost( threadarg->semaphore );
      } 
      /* Make a new thread */
      else if( SDL_SemTryWait(stopped->semaphore) == 0) {
         /* Assign arguments for the thread */
         threadarg = tq_dequeue(stopped);
         threadarg->function = node ->function;
         threadarg->data = node->data;
         threadarg->signal = THREADSIG_RUN;
         /* Signal the thread that there's a new job */
         SDL_SemPost( threadarg->semaphore );
         /* Start a new thread and increment the thread counter */
         SDL_CreateThread( threadpool_worker, threadarg );
         nrunning += 1;
      } 
      /* Wait for idle thread */
      else {
         while (SDL_SemWait(idle->semaphore) == -1) {
             /* Bad idea */
             WARN("L%d: SDL_SemWait failed! Error: %s", __LINE__, SDL_GetError());
         }
         /* Assign arguments for the thread */
         threadarg = tq_dequeue( idle );
         threadarg->function = node->function;
         threadarg->data = node->data;
         /* Signal the thread that there's a new job */
         SDL_SemPost( threadarg->semaphore );
      }

      /* Free the now unused job from the global_queue */
      free(node);
   }
   /* TODO: cleanup and a way to stop the threadpool */
   return 0;
}

/**
 * @brief Initialize the global threadpool.
 *
 * @return Returns 0 on success and -1 if there's already a threadpool.
 */
int threadpool_init()
{
   /* There's already a queue */
   if (global_queue != NULL) {
      WARN("Threadpool has already been initialized!");
      return -1;
   }

   /* Create the queue */
   global_queue = tq_create();

   /* make a threadpool_handler */
   SDL_CreateThread( threadpool_handler, NULL );

   return 0;
}

/**
 * @brief Creates a new vpool queue.
 * This is just an interface to make running a number of jobs and then wait for
 * them to finish more pleasant. You should not nest vpools as of now as there
 * are only a limit number of worker threads and we can't have them wait for a
 * thread to finish that doesn't exist. 
 * If you really want to sort of nest vpools, you should start a new thread
 * instead of using the threadpool. I might add a vpool_waitInANewThread
 * function some day.
 *
 * @return Returns a ThreadQueue to be used.
 */
ThreadQueue vpool_create()
{
   return tq_create();
}

/**
 * @brief Enqueue a job in the vpool queue.
 * @warning Do NOT enqueue jobs that wait for another job to be done, as this
 * could lead to a deadlock.
 * @warning Do NOT enqueue jobs that wait for a vpool, as this could lead to a
 * deadlock.
 */
void vpool_enqueue(ThreadQueue queue, int (*function)(void *), void *data)
{
   ThreadQueue_data node;
   
   node = calloc( 1, sizeof(ThreadQueue_data_) );
   node->data = data;
   node->function = function;
   
   tq_enqueue( queue, node );
}

/**
 * @brief A special vpool worker that signals the waiting thread when all jobs
 * are done.
 * It uses a mutex+condition variable+counter
 */
static int vpool_worker( void *data )
{
   vpoolThreadData work;
   
   work = (vpoolThreadData) data;

   /* Do work */
   work->node->function( work->node->data );

   /* Decrement the counter and signal vpool_wait if all threads are done */
   SDL_mutexP( work->mutex );
   *(work->count) = *(work->count) - 1; 
   if (*(work->count) == 0)             /* All jobs are done */
      SDL_CondSignal( work->cond );     /* Signal waiting thread */
   SDL_mutexV( work->mutex );

   return 0;
}

/* @brief Run every job in the vpool queue and block until every job in the
 * queue is done.
 * @note It destroys the queue when it's done.
 */
void vpool_wait(ThreadQueue queue)
{
   int i, cnt;
   SDL_cond *cond;
   SDL_mutex *mutex;
   vpoolThreadData arg;
   ThreadQueue_data node;

   cond = SDL_CreateCond();
   mutex = SDL_CreateMutex();
   /* This might be a little ugly (and inefficient?) */
   cnt = SDL_SemValue( queue->semaphore );

   /* Allocate all vpoolThreadData objects */
   arg = calloc( cnt, sizeof(vpoolThreadData_) );

   SDL_mutexP( mutex );
   /* Initialize the vpoolThreadData */
   for (i=0; i<cnt; i++) {
      /* This is needed to keep the invariants of the queue */
      while (SDL_SemWait( queue->semaphore ) == -1) {
          /* Again, a really bad idea */
          WARN("L%d: SDL_SemWait failed! Error: %s", __LINE__, SDL_GetError());
      }
      node = tq_dequeue( queue );

      arg[i].node = node;
      arg[i].cond = cond;
      arg[i].mutex = mutex;
      arg[i].count = &cnt;

      threadpool_newJob( vpool_worker, arg+i );
   }

   /* Wait for the threads to finish */
   SDL_CondWait( cond, mutex );
   SDL_mutexV( mutex );

   /* Clean up */
   SDL_DestroyMutex( mutex );
   SDL_DestroyCond( cond );
   tq_destroy( queue );
   free(arg);
}

/**
 * Notes
 *
 * The algorithm/strategy for killing idle workers should be moved into the
 * threadhandler and it should also be improved (the current strategy is
 * probably not very good).
 */
