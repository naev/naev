/*
 * See Licensing and Copyright notice in threadpool.h
 */
/*
 * @brief A simple threadpool implementation using a single queue.
 *
 * The queue is inspired by this paper (look for the queue with two locks):
 *
 * Maged M. Michael and Michael L. Scott. 1998. Nonblocking algorithms and
 * preemption-safe locking on multiprogrammed shared memory multiprocessors. J.
 * Parallel Distrib. Comput. 51, 1 (May 1998), 1-26. DOI=10.1006/jpdc.1998.1446
 * http://dx.doi.org/10.1006/jpdc.1998.1446
 *
 * @ARTICLE{Michael98non-blockingalgorithms,
 *    author = {Maged M. Michael and Michael L. Scott},
 *    title = {Non-Blocking Algorithms and Preemption-Safe Locking on Multiprogrammed Shared Memory Multiprocessors},
 *    journal = {Journal of Parallel and Distributed Computing},
 *    year = {1998},
 *    volume = {51},
 *    pages = {1--26},
 * }
 *
 * @note The algorithm/strategy for killing idle workers should be moved into
 *       the threadhandler and it should also be improved (the current strategy
 *       is probably not very good).
 */


#include "threadpool.h"

#include "SDL.h"
#include "SDL_thread.h"

#include <stdlib.h>

#include "log.h"


#define THREADPOOL_TIMEOUT (5 * 100) /* The time a worker thread waits in ms. */
#define THREADSIG_STOP     (1) /* The signal to stop a worker thread */
#define THREADSIG_RUN      (0) /* The signal to indicate the worker thread is running */


/**
 * Threads to use.
 */
static int MAXTHREADS = 8; /* Bit overkill, but oh well. */


/**
 * @brief Node in the thread queue.
 */
typedef struct Node_ {
   void *data;          /* The element in the list */
   struct Node_ *next;  /* The next node in the list */
} Node;

/**
 * @brief Threadqueue itself.
 */
struct ThreadQueue_ {
   Node *first;         /* The first node */
   Node *last;          /* The second node */
   /* A semaphore to ensure reads only happen when the queue is not empty */
   SDL_sem *semaphore;
   SDL_mutex *t_lock;   /* Tail lock. Lock when reading/updating tail */
   SDL_mutex *h_lock;   /* Same as tail lock, except it's head lock */
};

/**
 * @brief Data for the threadqueue.
 */
typedef struct ThreadQueueData_ {
   int (*function)(void *);  /* The function to be called */
   void *data;               /* And its arguments */
} ThreadQueueData;

/**
 * @brief Thread data.
 */
typedef struct ThreadData_ {
   int (*function)(void *); /* The function to be called */
   void *data;             /* Arguments to the above function */
   int signal;             /* Signals to the thread */
   SDL_sem *semaphore;     /* The semaphore to signal new jobs or new signal in the
                              'signal' variable */
   ThreadQueue *idle;      /* The queue with idle threads */
   ThreadQueue *stopped;   /* The queue with stopped threads */
} ThreadData;

/**
 * @brief Virtual thread pool data.
 */
typedef struct vpoolThreadData_ {
   SDL_cond *cond;         /* Condition variable for signalling all jobs in the vpool
                              are done */
   SDL_mutex *mutex;       /* The mutex to use with the above condition variable */
   int *count;             /* Variable to count number of finished jobs in the vpool */
   ThreadQueueData *node;  /* The job to be done */
} vpoolThreadData;

/* The global threadpool queue */
static ThreadQueue *global_queue = NULL;


/*
 * Prototypes.
 */
static ThreadQueue* tq_create (void);
static void tq_enqueue( ThreadQueue *q, void *data );
static void* tq_dequeue( ThreadQueue *q );
static void tq_destroy( ThreadQueue *q );
static int threadpool_worker( void *data );
static int threadpool_handler( void *data );
static int vpool_worker( void *data );


/**
 * @brief Creates a concurrent queue.
 *
 * The basic concept is that we have a separate lock for both head and tail, so
 *  we can operate separately on either head or tail. This lets us enqueue at
 *  tail and dequeue at the head in true FIFO fashion.
 *
 *    @return The ThreadQueue.
 */
static ThreadQueue* tq_create (void)
{
   ThreadQueue *q;
   Node *n;

   /* Queue memory allocation. */
   q        = calloc( 1, sizeof(ThreadQueue) );

   /* Allocate and insert the dummy node */
   n        = calloc( 1, sizeof(Node) );
   n->next  = NULL;
   q->first = n;
   q->last  = n;

   /* Create locks. */
   q->t_lock      = SDL_CreateMutex();
   q->h_lock      = SDL_CreateMutex();
   q->semaphore   = SDL_CreateSemaphore( 0 );

   return q;
}

/**
 * @brief Enqueue data to the ThreadQueue q.
 *
 *    @param q The queue to be inserted into.
 *    @param data The element to be stored in the queue.
 */
static void tq_enqueue( ThreadQueue *q, void *data )
{
   Node *n;

   /* Allocate new struct. */
   n        = calloc( 1, sizeof(Node) );
   n->data  = data;
   n->next  = NULL;

   /* Lock */
   SDL_mutexP( q->t_lock );

   /* Enqueue. */
   q->last->next  = n;
   q->last        = n;

   /* Signal and unlock. This wil break if someone tries to enqueue 2^32+1
    * elements or something. */
   SDL_SemPost( q->semaphore );
   SDL_mutexV( q->t_lock );
}

/**
 * @brief Dequeue from the ThreadQueue q.
 *
 * @attention The callee should ALWAYS have called SDL_SemWait() on the semaphore.
 *
 *    @param q The queue to dequeue from.
 *    @return A void pointer to the element from the queue.
 */
static void* tq_dequeue( ThreadQueue *q )
{
   void *d;
   Node *newhead, *node;

   /* Lock the head. */
   SDL_mutexP( q->h_lock );

   /* Start running. */
   node     = q->first;
   newhead  = node->next;

   /* Head not consistent. */
   if (newhead == NULL) {
      WARN("Tried to dequeue while the queue was empty!");
      /* Ugly fix :/ */
      /*
      SDL_mutexV(q->h_lock);
      return NULL;
      */
      /* We prefer to wait until the cache updates :/ */
      do {
         node     = q->first;
         newhead  = node->next;
      } while (newhead == NULL);
   }

   /* Remember the value and assign newhead as the new dummy element. */
   d        = newhead->data;
   q->first = newhead;

   /* Unlock */
   SDL_mutexV( q->h_lock );

   free( node );
   return d;
}

/**
 * @brief Destroys and frees a ThreadQueue.
 *
 * Frees all elements too.
 *
 *    @param q The ThreadQueue to free.
 */
static void tq_destroy( ThreadQueue *q )
{
   /* Iterate through the list and free the nodes */
   while (q->first->next != NULL)
      free( tq_dequeue(q) ); /* Locks q->t_lock, so we must destroy mutex after. */

   /* Clean up threading structures. */
   SDL_DestroySemaphore( q->semaphore );
   SDL_DestroyMutex( q->h_lock );
   SDL_DestroyMutex( q->t_lock );

   free( q->first );
   free( q );
}


/**
 * @brief Enqueues a new job for the threadpool.
 *
 * @warning Do NOT enqueue a job that has to wait for another job to be done as
 *          this could lead to a deadlock.
 *
 *    @param function The function (job) to be called (executed).
 *    @param data The arguments for the function.
 *    @return Returns 0 on success and -2 if there was no threadpool.
 */
int threadpool_newJob( int (*function)(void *), void *data )
{
   ThreadQueueData *node;

   if (global_queue == NULL) {
      WARN("Threadpool has not been initialized yet!");
      return -2;
   }

   /* Allocate and set parameters. */
   node           = calloc( 1, sizeof(ThreadQueueData) );
   node->data     = data;
   node->function = function;

   /* Actually enque. */
   tq_enqueue( global_queue, node );

   return 0;
}

/**
 * @brief The worker function for the threadpool.
 *
 * It waits for a signal from the handler. If it receives THREADSIG_STOP it
 *  means the worker thread should stop. Else it dequeues a job from the
 *  global_queue and executes it.
 *
 *    @param data A pointer to the ThreadData struct used for a lot of stuff.
 */
static int threadpool_worker( void *data )
{
   ThreadData *work;

   work = (ThreadData*) data;

   /* Work loop */
   while (1) {
      /* Wait for new signal */
      while (SDL_SemWait( work->semaphore ) == -1) {
          /* Putting this in a while-loop is probably a really bad idea, but I
           * don't have any better ideas. */
          WARN("SDL_SemWait failed! Error: %s", SDL_GetError());
      }
      /* Break if received signal to stop */
      if (work->signal == THREADSIG_STOP)
         break;

      /* Do work :-) */
      work->function( work->data );

      /* Enqueue itself in the idle worker threads queue */
      tq_enqueue( work->idle, work );
   }
   /* Enqueue itself in the stopped worker threads queue when stopped */
   tq_enqueue( work->stopped, work );

   return 0;
}

/**
 * @brief Handles assigning jobs to the workers and killing them if necessary.
 *
 * @note Stopping the threeadpool_handler is not yet implemented.
 *
 * Process is:
 *
 * 1) Wait for job
 * 2) if Timed out -> kill idle, putting on stopped, go to 1)
 * 3) Have thread run job
 * 3.1) Grab a job from the queue
 * 3.2) If idle thread, grab idle thread
 *      else if stopped threads, reactivate stopped thread
 *      else wait for running thread
 * 4) Go to 1)
 *
 *    @param data Not used. SDL threading requires functions to take a void
 *                pointer as argument.
 *    @return Not really implemented yet.
 */
static int threadpool_handler( void *data )
{
   (void) data;
   int i, nrunning, newthread;
   ThreadData *threadargs, *threadarg;
   /* Queues for idle workers and stopped workers */
   ThreadQueue *idle, *stopped;
   ThreadQueueData *node;

   /* Initialize the idle and stopped queues. */
   idle     = tq_create();
   stopped  = tq_create();

   /* Allocate threadargs to communicate with workers */
   threadargs = calloc( MAXTHREADS, sizeof(ThreadData) );

   /* Initialize threadargs */
   for (i=0; i<MAXTHREADS; i++) {
      threadargs[i].function  = NULL;
      threadargs[i].data      = NULL;
      threadargs[i].semaphore = SDL_CreateSemaphore( 0 ); /* Used to give orders. */
      threadargs[i].idle      = idle;
      threadargs[i].stopped   = stopped;
      threadargs[i].signal    = THREADSIG_RUN;
      /* 'Workers' that do not have a thread are considered stopped */
      tq_enqueue( stopped, &threadargs[i] );
   }

   /* Set the number of running threads to 0 */
   nrunning = 0;

   /*
    * Thread handler main loop.
    */
   while (1) {
      /*
       * We must now wait, this shall be done on each active thread. However they will
       * be put to sleep as time passes. When we receive a command we'll proceed to process
       * it.
       */
      if (nrunning > 0) {
         /*
          * Here we'll wait until thread gets work to do. If it doesn't it will
          * just stop a worker thread and wait until it gets something to do.
          */
         if (SDL_SemWaitTimeout( global_queue->semaphore, THREADPOOL_TIMEOUT ) != 0) {
            /* There weren't any new jobs so we'll start killing threads ;) */
            if (SDL_SemTryWait( idle->semaphore ) == 0) {
               threadarg         = tq_dequeue( idle );
               /* Set signal to stop worker thread */
               threadarg->signal = THREADSIG_STOP;
               /* Signal thread and decrement running threads counter */
               SDL_SemPost( threadarg->semaphore );
               nrunning -= 1;
            }

            /* We just go back to waiting on a thread. */
            continue;
         }

         /* We got work. Continue to handle work. */
      }
      else {
         /*
          * Here we wait for a new job. No threads are alive at this point and the
          * threadpool is just patiently waiting for work to arrive.
          */
         if (SDL_SemWait( global_queue->semaphore ) == -1) {
             WARN("SDL_SemWait failed! Error: %s", SDL_GetError());
             continue;
         }

         /* We got work. Continue to handle work. */
      }

      /*
       * Get a new job from the queue. This should be safe as we have received
       * a permission from the global_queue->semaphore.
       */
      node        = tq_dequeue( global_queue );
      newthread   = 0;

      /*
       * Choose where to get the thread. Either idle, revive stopped or block until
       * another thread becomes idle.
       */
      /* Idle thread available */
      if (SDL_SemTryWait(idle->semaphore) == 0)
         threadarg         = tq_dequeue( idle );
      /* Make a new thread */
      else if (SDL_SemTryWait(stopped->semaphore) == 0) {
         threadarg         = tq_dequeue( stopped );
         threadarg->signal = THREADSIG_RUN;
         newthread         = 1;
      }
      /* Wait for idle thread */
      else {
         while (SDL_SemWait(idle->semaphore) == -1) {
             /* Bad idea */
             WARN("SDL_SemWait failed! Error: %s", SDL_GetError());
         }
         threadarg         = tq_dequeue( idle );
      }

      /* Assign arguments for the thread */
      threadarg->function  = node->function;
      threadarg->data      = node->data;
      /* Signal the thread that there's a new job */
      SDL_SemPost( threadarg->semaphore );

      /* Start a new thread and increment the thread counter */
      if (newthread) {
         SDL_CreateThread( threadpool_worker,
#if SDL_VERSION_ATLEAST(1,3,0)
               "threadpool_worker",
#endif /* SDL_VERSION_ATLEAST(1,3,0) */
               threadarg );
         nrunning += 1;
      }

      /* Free the now unused job from the global_queue */
      free(node);
   }
   /** @TODO A way to stop the threadpool. */

   /* Clean up. */
   tq_destroy( idle );
   tq_destroy( stopped );
   free( threadargs );

   return 0;
}

/**
 * @brief Initialize the global threadpool.
 *
 *    @return Returns 0 on success and -1 if there's already a threadpool.
 */
int threadpool_init (void)
{
#if SDL_VERSION_ATLEAST(1,3,0)
   MAXTHREADS = SDL_GetCPUCount() + 1; /* SDL 1.3 is pretty cool. */
#endif /* SDL_VERSION_ATLEAST(1,3,0) */

   /* There's already a queue */
   if (global_queue != NULL) {
      WARN("Threadpool has already been initialized!");
      return -1;
   }

   /* Create the global queue queue */
   global_queue = tq_create();

   /* Initialize the threadpool handler. */
   SDL_CreateThread( threadpool_handler,
#if SDL_VERSION_ATLEAST(1,3,0)
               "threadpool_handler",
#endif /* SDL_VERSION_ATLEAST(1,3,0) */
               NULL );

   return 0;
}

/**
 * @brief Creates a new vpool queue.
 *
 * This is just an interface to make running a number of jobs and then wait for
 *  them to finish more pleasant. You should not nest vpools as of now as there
 *  are only a limit number of worker threads and we can't have them wait for a
 *  thread to finish that doesn't exist.
 *
 * If you really want to sort of nest vpools, you should start a new thread
 *  instead of using the threadpool. I might add a vpool_waitInANewThread
 *  function some day.
 *
 *    @return Returns a ThreadQueue to be used.
 */
ThreadQueue* vpool_create (void)
{
   return tq_create();
}

/**
 * @brief Enqueue a job in the vpool queue.
 *
 * @warning Do NOT enqueue jobs that wait for another job to be done, as this
 *          could lead to a deadlock.
 *
 * @warning Do NOT enqueue jobs that wait for a vpool, as this could lead to a
 *          deadlock.
 */
void vpool_enqueue( ThreadQueue *queue, int (*function)(void *), void *data )
{
   ThreadQueueData *node;

   /* Allocate and set up data. */
   node           = calloc( 1, sizeof(ThreadQueueData) );
   node->data     = data;
   node->function = function;

   /* Add to vpool. */
   tq_enqueue( queue, node );
}

/**
 * @brief A special vpool worker that signals the waiting thread when all jobs
 *        are done.
 *
 * It uses a mutex+condition variable+counter.
 */
static int vpool_worker( void *data )
{
   int cnt;
   vpoolThreadData *work;

   work = (vpoolThreadData*) data;

   /* Do work */
   work->node->function( work->node->data );

   /* Decrement the counter and signal vpool_wait if all threads are done */
   SDL_mutexP( work->mutex );
   cnt   = *(work->count) - 1;
   if (cnt <= 0)                    /* All jobs done. */
      SDL_CondSignal( work->cond );  /* Signal waiting thread */
   *(work->count) = cnt;
   SDL_mutexV( work->mutex );

   return 0;
}

/* @brief Run every job in the vpool queue and block until every job in the
 *        queue is done.
 *
 * @note It destroys the queue when it's done.
 */
void vpool_wait( ThreadQueue *queue )
{
   int i, cnt;
   SDL_cond *cond;
   SDL_mutex *mutex;
   vpoolThreadData *arg;
   ThreadQueueData *node;

   /* Create temporary threading structures. */
   cond  = SDL_CreateCond();
   mutex = SDL_CreateMutex();
   /* This might be a little ugly (and inefficient?) */
   cnt   = SDL_SemValue( queue->semaphore );

   /* Allocate all vpoolThreadData objects */
   arg = calloc( cnt, sizeof(vpoolThreadData) );

   SDL_mutexP( mutex );
   /* Initialize the vpoolThreadData */
   for (i=0; i<cnt; i++) {
      /* This is needed to keep the invariants of the queue */
      while (SDL_SemWait( queue->semaphore ) == -1) {
          /* Again, a really bad idea */
          WARN("SDL_SemWait failed! Error: %s", SDL_GetError());
      }
      node = tq_dequeue( queue );

      /* Set up arguments. */
      arg[i].node    = node;
      arg[i].cond    = cond;
      arg[i].mutex   = mutex;
      arg[i].count   = &cnt;

      /* Launch new job. */
      threadpool_newJob( vpool_worker, &arg[i] );
   }

   /* Wait for the threads to finish */
   SDL_CondWait( cond, mutex );
   SDL_mutexV( mutex );

   /* Clean up */
   SDL_DestroyMutex( mutex );
   SDL_DestroyCond( cond );
   tq_destroy( queue );
   free( arg );
}


