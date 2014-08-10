/*
 * Copyright 2006-2012 Edgar Simo Serra
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

/**
 * @file queue.c
 *
 * @brief A dodgy implementation of a queue.
 */


#include "queue.h"

#include <stdlib.h>

#include "log.h"


/**
 * @brief Node struct.
 */
typedef struct Node_ *Node;
typedef struct Node_ {
   void *data; /**< Assosciated data. */
   Node next; /**< Next node. */
} Node_;

/**
 * @brief Queue struct.
 */
typedef struct Queue_ {
   Node first; /**< First node in the queue. */
   Node last; /**< Last node in the queue. */
} Queue_;

/**
 * @brief Creates a queue.
 *
 *    @return A pointer to a queue.
 */
Queue q_create (void)
{
   /* Create the queue. */
   Queue q = malloc(sizeof(Queue_));

   /* Check that we didn't get a NULL. */
#ifdef DEBUGGING
   if (q == NULL) {
      WARN("q == NULL");
      return NULL;
   }
#endif /* DEBUGGING */

   /* Assign nothing into it. */
   q->first = NULL;
   q->last  = NULL;

   /* And return (a pointer to) the newly created queue. */
   return q;
}

/**
 * @brief Destroys a queue.
 *
 *    @param q Queue to destroy.
 */
void q_destroy( Queue q )
{
#ifdef DEBUGGING
   /* Check that we didn't get a NULL. */
   if (q == NULL) {
      WARN("q == NULL");
      return;
   }
#endif /* DEBUGGING */

   /* Free all the data. */
   while(q->first != NULL)
      q_dequeue(q);

   free(q);

   return;
}

/**
 * @brief Enqueues an item.
 *
 *    @param q Queue to use.
 *    @param data Item to enqueue.
 */
void q_enqueue( Queue q, void *data )
{
   Node n;

#ifdef DEBUGGING
   /* Check that we didn't get a NULL. */
   if (q == NULL) {
      WARN("q == NULL");
      return;
   }
#endif /* DEBUGGING */

   /* Create a new node. */
   n = malloc(sizeof(Node_));
   n->data = data;
   n->next = NULL;
   if (q->first == NULL)
      q->first = n;
   else
      q->last->next = n;
   q->last = n;

   return;
}

/**
 * @brief Dequeues an item.
 *
 *    @param q Queue to use.
 *    @return The data.
 */
void* q_dequeue( Queue q )
{
   void *d;
   Node temp;

#ifdef DEBUGGING
   /* Check that we didn't get a NULL. */
   if (q == NULL) {
      WARN("q == NULL");
      return NULL;
   }
#endif /* DEBUGGING */

   /* Check that it's not empty. */
   if (q->first == NULL)
      return NULL;

   d        = q->first->data;
   temp     = q->first;
   q->first = q->first->next;
   if (q->first == NULL)
      q->last = NULL;
   free(temp);

   return d;
}

/**
 * @brief Checks if the queue is empty.
 *
 *    @param q Queue to use.
 *    @return 1 if it's empty, 0 if it has data.
 */
int q_isEmpty( Queue q )
{
#ifdef DEBUGGING
   /* Check that we didn't get a NULL. */
   if (q == NULL) {
      WARN("q == NULL");
      return -1;
   }
#endif /* DEBUGGING */

   if (q->first == NULL)
      return 1;
   else
      return 0;
}
