#include "array.h"

#include <stdlib.h>
#include <stdio.h>

#include "nstring.h"

void *_array_create_helper(size_t e_size)
{
   _private_container *c = malloc(sizeof(_private_container) + e_size);
#ifdef DEBUGGING
   c->_sentinel = SENTINEL;
#endif
   c->_reserved = 1;
   c->_size = 0;
   return c->_array;
}

static void _array_resize_container(_private_container **c_, size_t e_size, int new_size)
{
   assert( new_size >= 0 );
   _private_container *c = *c_;

   if (new_size > c->_reserved) {
      /* increases the reserved space */
      do
         c->_reserved *= 2;
      while (new_size < c->_reserved);

      c = realloc(c, sizeof(_private_container) + e_size * c->_reserved);
   }

   c->_size = new_size;
   *c_ = c;
}

void _array_resize_helper(void **a, size_t e_size, int new_size)
{
   _private_container *c = _array_private_container(*a);
   _array_resize_container(&c, e_size, new_size);
   *a = c->_array;
}

void *_array_grow_helper(void **a, size_t e_size)
{
   _private_container *c = _array_private_container(*a);
   if (c->_size == c->_reserved) {
      /* Array full, doubles the reserved memory */
      c->_reserved *= 2;
      c = realloc(c, sizeof(_private_container) + e_size * c->_reserved);
      *a = c->_array;
   }

   return c->_array + (c->_size++) * e_size;
}

void _array_erase_helper(void **a, size_t e_size, void *first, void *last)
{
   intptr_t diff = (char *)last - (char *)first;

   /* copies the memory */
   _private_container *c = _array_private_container(*a);
   char *end = c->_array + c->_size * e_size;
   memmove(first, last, end - (char *)last);

   /* resizes the array */
   assert("Invalid iterators passed to array erase" && (diff % e_size == 0));
   c->_size -= diff / e_size;
}

void _array_shrink_helper(void **a, size_t e_size)
{
   _private_container *c = _array_private_container(*a);
   if (c->_size != 0) {
      c = realloc(c, sizeof(_private_container) + e_size * c->_size);
      c->_reserved = c->_size;
   } else {
      c = realloc(c, sizeof(_private_container) + e_size);
      c->_reserved = 1;
   }
   *a = c->_array;
}

void _array_free_helper(void *a)
{
   free(_array_private_container(a));
}

#if 0
int main() {
   const int size = 100;

   int i;
   int *array = array_create(int);

   /* pushes some elements */
   for (i = 0; i < size; ++i)
      array_push_back(&array, i);
   assert(array_size(array) == size);
   assert(array_size(array) <= array_reserved(array));
   for (i = 0; i < size; ++i)
      assert(array[i] == i);

   /* erases second half */
   array_erase(&array, array + size / 2, array + size);
   assert(array_size(array) == size / 2);
   assert(array_size(array) <= array_reserved(array));
   for (i = 0; i < size / 2; ++i)
      assert(array[i] == i);

   /* shrinks */
   array_shrink(&array);
   assert(array_size(array) == array_reserved(array));

   /* pushes back second half */
   for (i = size / 2; i < size; ++i)
      array_push_back(&array, i);
   assert(array_size(array) == size);
   assert(array_size(array) <= array_reserved(array));
   for (i = 0; i < size; ++i)
      assert(array[i] == i);

   /* erases middle half */
   array_erase(&array, array + size / 4, array + 3 * size / 4);
   assert(array_size(array) == size / 2);
   assert(array_size(array) <= array_reserved(array));
   for (i = 0; i < size / 4; ++i)
      assert(array[i] == i);
   for (; i < size / 2; ++i)
      assert(array[i] == i + size / 2);

   /* shrinks */
   array_shrink(&array);
   assert(array_size(array) == array_reserved(array));

   /* erases one element */
   array_erase(&array, array, array + 1);
   assert(array_size(array) == size / 2 - 1);
   for (i = 1; i < size / 4; ++i)
      assert(array[i - 1] == i);
   for (; i < size / 2; ++i)
      assert(array[i - 1] == i + size / 2);

   /* erases no elements */
   array_erase(&array, array, array);
   array_erase(&array, array + array_size(array), array + array_size(array));
   assert(array_size(array) == size / 2 - 1);
   for (i = 1; i < size / 4; ++i)
      assert(array[i - 1] == i);
   for (; i < size / 2; ++i)
      assert(array[i - 1] == i + size / 2);

   /* erases all elements */
   array_erase(&array, array, array + array_size(array));
   assert(array_size(array) == 0);

   /* shrinks */
   array_shrink(&array);
   assert(array_size(array) == 0);
   assert(array_reserved(array) == 1);

   return 0;
}
#endif
