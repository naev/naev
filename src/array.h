/**< Provides macros to work with dynamic arrays.
 * NOTE: Except were noted, macros do not have side effects from
 * expations */

#ifndef ARRAY_H
#  define ARRAY_H

#include <stddef.h>
#include <assert.h>
#include <stdint.h>

#ifdef DEBUGGING
#define SENTINEL ((int)0xbabecafe)
#endif

typedef struct {
#ifdef DEBUGGING
   int _sentinel;
#endif
   int _reserved;         /**< Number of elements reserved */
   int _size;             /**< Number of elements in the array */
   char _array[1];        /**< Begin of the array */
} _private_container;


void *_array_create_helper(size_t e_size);
void *_array_grow_helper(void **a, size_t e_size);
void _array_shrink_helper(void **a, size_t e_size);
void _array_free_helper(void *a);

__inline__ static _private_container *_array_private_container(void *a)
{
   assert("NULL array!" && (a != NULL));

   const intptr_t delta = (intptr_t)(&((_private_container *)NULL)->_array);
   _private_container *c = (_private_container *)((char *)a - delta);

#ifdef DEBUGGING
   assert("Sentinel not found. Use array_create() to create the array." && (c->_sentinel == SENTINEL));
#endif

   return c;
}

__inline__ static void *_array_end_helper(void *a)
{
   _private_container *c = _array_private_container(a);
   return c->_array + c->_size;
}

/** @brief Creates a new dynamic array of `basic_type' */
#define array_create(basic_type) \
      ((basic_type *)(_array_create_helper(sizeof(basic_type))))

/** @brief Increases the number of elements by one and returns the last element.
 * NOTE: Invalidates all iterators. */
#define array_grow(ptr_array) \
      (*(__typeof__((ptr_array)[0]))_array_grow_helper((void **)(ptr_array), sizeof((ptr_array)[0][0])))
/** @brief Shrinks memory to fit only `size' elements.
 * NOTE: Invalidates all iterators. */
#define array_shrink(ptr_array) \
      (_array_shrink_helper((void **)(ptr_array), sizeof((ptr_array)[0][0])))
/** @brief Frees memory allocated and sets array to NULL.
 * NOTE: Invalidates all iterators. */
#define array_free(array) \
      _array_free_helper((void *)(array))

/** @brief Returns number of elements in the array */
#define array_size(array) (_array_private_container(array)->_size)
/** @brief Returns number of elements reserved */
#define array_reserved(array) (_array_private_container(array)->_reserved)
/** @brief Returns a pointer to the begining of the reserved memory space */
#define array_begin(array) (array)
/** @brief Returns a pointer to the end of the reserved memory space */
#define array_end(array) ((__typeof__(array))_array_end_helper(array))
/** @brief Returns the first element in the array */
#define array_front(a) (*array_begin(a))
/** @brief Returns the last element in the array */
#define array_back(a) (*(array_end(a) - 1))

#endif /* ARRAY_H */
