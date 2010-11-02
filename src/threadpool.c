/*
 * See Licensing and Copyright notice in threadpool.h
 */
/*
 * The queue is inspired by this paper (look for the queue with two locks):
 * http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.109.5602&rep=rep1&type=pdf
 */

#include "log.h"

#include "SDL.h"
#include "SDL_thread.h"

#include <stdlib.h>

#define THREADPOOL_TIMEOUT (5 * 100)
#define THREADSIG_STOP (1)
#define THREADSIG_RUN (0)


#if SDL_VERSION_ATLEAST(1,3,0)
const int MAXTHREADS = SDL_GetCPUCount();
#else
const int MAXTHREADS = 8;
#endif


typedef struct Node_ *Node;
typedef struct Node_ {
   void *data;
   Node next;
} Node_;

typedef struct ThreadQueue_ {
   Node first;
   Node last;
   SDL_sem *semaphore; 
   SDL_mutex *t_lock;
   SDL_mutex *h_lock;
} ThreadQueue_;
typedef ThreadQueue_ *ThreadQueue;

typedef struct ThreadQueue_data_ {
   int (*function)(void *);
   void *data;
} ThreadQueue_data_;
typedef ThreadQueue_data_ *ThreadQueue_data;

typedef struct ThreadData_ {
   int (*function)(void *);
   void *data;
   int signal;
   SDL_sem *semaphore;
   ThreadQueue idle;
   ThreadQueue stopped;
} ThreadData_;

typedef struct vpoolThreadData_ {
   SDL_cond *cond;
   SDL_mutex *mutex;
   int *count;
   ThreadQueue_data node;
} vpoolThreadData_;
typedef vpoolThreadData_ *vpoolThreadData;


static ThreadQueue queue = NULL;

ThreadQueue tq_create()
{
   Node n;
   ThreadQueue q;

   q = malloc( sizeof(ThreadQueue_) );

   /* Allocate and insert the dummy node */
   n = malloc( sizeof(Node_) );
   n->next = NULL;
   q->first = n;
   q->last = n;

   q->t_lock = SDL_CreateMutex();
   q->h_lock = SDL_CreateMutex();
   q->semaphore = SDL_CreateSemaphore( 0 );

   return q;
}


void tq_enqueue( ThreadQueue q, void *data )
{
   Node n;

   n = malloc(sizeof(Node_));
   n->data = data;
   n->next = NULL;

   /* Lock */
   SDL_mutexP(q->t_lock);

   q->last->next = n;
   q->last = n;

   /* Signal and unlock.
    * This wil break if someone tries to enqueue 2^32+1 elements or something.
    * */
   SDL_SemPost(q->semaphore);
   SDL_mutexV(q->t_lock);
}

/* IMPORTANT! The callee should ALWAYS have called SDL_SemWait() on the semaphore. */
void* tq_dequeue( ThreadQueue q )
{
   void *d;
   Node newhead, node;
   
   /* Lock */
   SDL_mutexP(q->h_lock);

   node = q->first;
   newhead = node->next;

   if (newhead == NULL) {
      #ifdef LOG_H
      WARN("Tried to dequeue while the queue was empty!");
      #endif
      /* Unlock and return NULL */
      SDL_mutexV(q->h_lock);
      return NULL;
   }
   /* Remember the value and assign newhead as the new dummy element. */
   d = newhead->data;
   q->first = newhead;
   
   SDL_mutexV(q->h_lock);
   free(node);
   return d;
}

/* This does not try to lock or anything */
void tq_destroy( ThreadQueue q )
{
   SDL_DestroySemaphore(q->semaphore);
   SDL_DestroyMutex(q->h_lock);
   SDL_DestroyMutex(q->t_lock);

   /* Shouldn't it free the dequeued nodes? */
   while(q->first->next != NULL) {
      free( tq_dequeue(q) );
   }
   
   free( q->first );
   free(q);
}

/* Eh, threadsafe? nah */
int tq_isEmpty( ThreadQueue q )
{
   if (q->first->next == NULL)
      return 1;
   else
      return 0;
}


/* Enqueues a new job for the threadpool. Do NOT enqueue a job that has to wait
 * for another job to be done as this could lead to a deadlock. */
int threadpool_newJob(int (*function)(void *), void *data)
{
   ThreadQueue_data node;
   if (queue == NULL) {
      #ifdef LOG_H
      WARN("threadpool.c: Threadpool has not been initialized yet!");
      #endif
      return -2;
   }
   
   node = malloc( sizeof(ThreadQueue_data_) );
   node->data = data;
   node->function = function;

   tq_enqueue( queue, node );
   
   return 0;
}

int threadpool_worker( void *data )
{
   ThreadData_ *work;
   
   work = (ThreadData_ *) data;

   while (1) {
      /* Break if signal to stop */
      SDL_SemWait( work->semaphore );
      if ( work->signal == THREADSIG_STOP ) {
         break;
      }

      /* Do work :-) */
      (*work->function)( work->data );
      
      tq_enqueue( work->idle, work );
   }
   tq_enqueue( work->stopped, work );

   return 0;
}

int threadpool_handler( void *data )
{
   int i, nrunning;
   ThreadData_ *threadargs, *threadarg;
   /* Queues for idle workers and stopped workers */
   ThreadQueue idle, stopped;
   ThreadQueue_data node;

   threadargs = malloc( sizeof(ThreadData_)*MAXTHREADS );

   idle = tq_create();
   stopped = tq_create();

   /* Initialize threadargs */
   for (i=0; i<MAXTHREADS; i++) {
      threadargs[i].function = NULL;
      threadargs[i].data = NULL;
      threadargs[i].semaphore = SDL_CreateSemaphore( 0 );
      threadargs[i].idle = idle;
      threadargs[i].stopped = stopped;
      threadargs[i].signal = THREADSIG_RUN;

      tq_enqueue(stopped, &threadargs[i]);
   }

   nrunning = 0;

   while (1) {

      if (nrunning > 0) {
         if (SDL_SemWaitTimeout( queue->semaphore, THREADPOOL_TIMEOUT ) != 0) {
            /* Start killing threads ;) */
            if ( SDL_SemTryWait(idle->semaphore) == 0 ) {
               threadarg = tq_dequeue( idle );
               threadarg->signal = THREADSIG_STOP;
               SDL_SemPost( threadarg->semaphore );
            }
            continue;
         }
      } else {
         /* Wait for a new job */
         SDL_SemWait( queue->semaphore );
      }
      node = tq_dequeue( queue );

      if( SDL_SemTryWait(idle->semaphore) == 0) {
         /* Idle thread available */
         threadarg = tq_dequeue( idle );
         threadarg->function = node->function;
         threadarg->data = node->data;
         SDL_SemPost( threadarg->semaphore );

      } else if( SDL_SemTryWait(stopped->semaphore) == 0) {
         /* Make new thread */
         threadarg = tq_dequeue(stopped);
         threadarg->function = node ->function;
         threadarg->data = node->data;
         threadarg->signal = THREADSIG_RUN;
         SDL_SemPost( threadarg->semaphore );

         SDL_CreateThread( threadpool_worker, threadarg );

      } else {
         /* Wait for idle thread */
         SDL_SemWait(idle->semaphore);
         threadarg = tq_dequeue( idle );
         threadarg->function = node->function;
         threadarg->data = node->data;
         SDL_SemPost( threadarg->semaphore );
      }
      free(node);
   }
   /* TODO: cleanup and maybe a way to stop the threadpool */
}

int threadpool_init()
{
   if (queue != NULL) {
      #ifdef LOG_H
      WARN("Threadpool has already been initialized!");
      #endif
      return -1;
   }

   queue = tq_create();

   SDL_CreateThread( threadpool_handler, NULL );

   return 0;
}

/* Creates a new vpool queue */
ThreadQueue vpool_create()
{
   return tq_create();
}

/* Enqueue a job in the vpool queue. Do NOT enqueue a job that has to wait for
 * another job to be done as this could lead to a deadlock. */
void vpool_enqueue(ThreadQueue queue, int (*function)(void *), void *data)
{
   ThreadQueue_data node;
   
   node = malloc( sizeof(ThreadQueue_data_) );
   node->data = data;
   node->function = function;
   
   tq_enqueue( queue, node );
}

int vpool_worker(void *data)
{
   vpoolThreadData work;
   
   work = (vpoolThreadData) data;

   /* Do work */
   work->node->function( work->node->data );

   /* Decrement the counter and signal vpool_wait if all threads are done */
   SDL_mutexP( work->mutex );
   *(work->count) = *(work->count) - 1;
   if (*(work->count) == 0)
      SDL_CondSignal( work->cond );
   SDL_mutexV( work->mutex );

   /* Cleanup */
   //free(work->node);
   //free(work);

   return 0;
}

/* Run every job in the vpool queue and block untill every job in the queue is
 * done. It destroys the queue when it's done. */
void vpool_wait(ThreadQueue queue)
{
   int i, cnt;
   SDL_cond *cond;
   SDL_mutex *mutex;
   vpoolThreadData arg;
   ThreadQueue_data node;

   cond = SDL_CreateCond();
   mutex = SDL_CreateMutex();
   cnt = SDL_SemValue( queue->semaphore );

   SDL_mutexP( mutex );
   arg = malloc( sizeof(vpoolThreadData_) * cnt );
   for (i=0; i<cnt; i++) {
      SDL_SemWait( queue->semaphore );
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

/* Notes
 *
 * The algorithm/strategy for killing idle workers should be moved into the
 * threadhandler and it should also be improved (the current strategy is
 * probably not very good).
 */
