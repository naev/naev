#include "array.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void *_array_create_helper(size_t e_size)
{
   _private_container *c = malloc(sizeof(_private_container) - 1 + e_size);
#ifdef DEBUG
   c->_sentinel = SENTINEL;
#endif
   c->_reserved = 1;
   c->_size = 0;
   return c->_array;
}

void *_array_grow_helper(void **a, size_t e_size) {
   _private_container *c = _array_private_container(*a);
   if (c->_size == c->_reserved) {
      /* Array full, doubles the reserved memory */
      c->_reserved *= 2;
      c = realloc(c, sizeof(_private_container) - 1 + e_size * c->_reserved);
      *a = c->_array;
   }

   return c->_array + (c->_size++) * e_size;
}

void _array_shrink_helper(void **a, size_t e_size)
{
   _private_container *c = _array_private_container(*a);
   c = realloc(c, sizeof(_private_container) - 1 + e_size * c->_size);
   c->_reserved = c->_size;
   *a = c->_array;
}

void _array_free_helper(void *a)
{
   free(_array_private_container(a));
}

#if 0
int main() {
   int *array = array_create(int);
   int i;

   for (i = 0; i < 100000000; ++i)
      array_grow(&array) = i;

   printf("%d\n", array_size(array));

   printf("%d\n", array_reserved(array));
   array_shrink(&array);
   printf("%d\n", array_reserved(array));

   for (i = 0; i < 100; ++i)
      printf("%d ", array[i]);
}
#endif
