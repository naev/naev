/* Copyright 2010 Reynir Reynisson
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef THREADPOOL_H
#  define THREADPOOL_H


struct ThreadQueue_;
typedef struct ThreadQueue_ ThreadQueue;


/* Initializes the threadpool */
int threadpool_init( void );

/* Enqueues a new job */
int threadpool_newJob( int (*function)(void *), void *data );

/* Creates a new vpool queue */
ThreadQueue* vpool_create( void );

/* Enqueue a job in the vpool queue. Do NOT enqueue a job that has to wait for
 * another job to be done as this could lead to a deadlock. */
void vpool_enqueue( ThreadQueue* queue, int (*function)(void *), void *data );

/* Run every job in the vpool queue and block until every job in the queue is
 * done. It destroys the queue when it's done. */
void vpool_wait( ThreadQueue* queue );



#endif
