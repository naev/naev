/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

typedef struct Queue_ *Queue;

Queue q_create( void );
void  q_destroy( Queue q );
void  q_enqueue( Queue q, void *data );
void *q_dequeue( Queue q );
int   q_isEmpty( Queue q );
