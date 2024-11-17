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
#pragma once

struct ThreadQueue_;
typedef struct ThreadQueue_ ThreadQueue;

/* Initializes the threadpool */
int threadpool_init( void );

/* Creates a new vpool queue. Destroy with vpool_wait. */
ThreadQueue *vpool_create( void );

/* Enqueue a job in the vpool queue. Do NOT enqueue a job that has to wait for
 * another job to be done as this could lead to a deadlock. Also do not enqueue
 * jobs from enqueued threads. */
void vpool_enqueue( ThreadQueue *queue, int ( *function )( void * ),
                    void        *data );

/* Run every job in the vpool queue and block until every job in the queue is
 * done. */
void vpool_wait( ThreadQueue *queue );

/* Clean up. */
void vpool_cleanup( ThreadQueue *queue );
