/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file array.h
 *
 * @brief Provides macros to work with dynamic arrays.
 *
 * @note Except were noted, macros do not have side effects from
 * expansions.
 *
 * Usage example:
 *
 * @code
 * static my_type *my_array = NULL;
 *
 * // Create array
 * my_array = array_create( my_type );
 *
 * // Fill array
 * while (need_fill)
 *    need_fill = fill_array_member( &array_grow( &my_array ) );
 *
 * // Shrink to minimum (if static it's a good idea).
 * array_shrink( &my_array );
 *
 * // Do stuff
 * for (i=0; i<array_size( my_array ); i++)
 *    do_stuff( &my_array[i] );
 *
 * // Clean up
 * array_free( my_array );
 * my_array = NULL;
 * @endcode
 */
#pragma once

/** @cond */
#include <assert.h>
#include <stddef.h>
#include <stdalign.h>
#include <stdint.h>
/** @endcond */

#include "attributes.h"

#define ARRAY_SENTINEL 0x15bada55 /**< Badass sentinel. */

/**
 * @brief Private container type for the arrays.
 */
typedef struct {
#if DEBUG_ARRAYS
   int _sentinel;         /**< Sentinel for when debugging. */
#endif /* DEBUG_ARRAYS */
   size_t _reserved;      /**< Number of elements reserved */
   size_t _size;          /**< Number of elements in the array */
   char alignas(max_align_t) _array[];  /**< Begin of the array */
} _private_container;

void *_array_create_helper(size_t e_size, size_t initial_size);
void *_array_grow_helper(void **a, size_t e_size);
void _array_resize_helper(void **a, size_t e_size, size_t new_size);
void _array_erase_helper(void **a, size_t e_size, void *first, void *last);
void _array_shrink_helper(void **a, size_t e_size);
void _array_free_helper(void *a);
void *_array_copy_helper(size_t e_size, void *a);

/**
 * @brief Gets the container of an array.
 *
 *    @param a Array to get container of.
 *    @return The container of the array a.
 */
static inline _private_container *_array_private_container(void *a)
{
   assert("NULL array!" && (a != NULL));

   _private_container *c = (_private_container *)a - 1;

#if DEBUG_ARRAYS
   assert("Sentinel not found. Use array_create() to create the array." && (c->_sentinel == ARRAY_SENTINEL));
#endif /* DEBUG_ARRAYS */

   return c;
}

/**
 * @brief Creates a new dynamic array of `basic_type'
 *
 *    @param basic_type Type of the array to create.
 */
#define array_create(basic_type) \
      ((basic_type *)(_array_create_helper(sizeof(basic_type), 1)))

/**
 * @brief Creates a new dynamic array of `basic_type' with an initial capacity
 *
 *    @param basic_type Type of the array to create.
 *    @param capacity Initial size.
 */
#define array_create_size(basic_type, capacity) \
      ((basic_type *)(_array_create_helper(sizeof(basic_type), capacity)))
/**
 * @brief Resizes the array to accomodate new_size elements.
 *
 * @note Invalidates all iterators.
 *
 *    @param ptr_array Array being manipulated.
 *    @param new_size New size to grow to (in number of elements).
 */
#define array_resize(ptr_array, new_size) \
   (_array_resize_helper((void **)(ptr_array), sizeof((ptr_array)[0][0]), new_size))
/**
 * @brief Increases the number of elements by one and returns the last element.
 *
 * @note Invalidates all iterators.
 */
#define array_grow(ptr_array) \
      (*(__typeof__((ptr_array)[0]))_array_grow_helper((void **)(ptr_array), sizeof((ptr_array)[0][0]))) // NOLINT
/**
 * @brief Adds a new element at the end of the array.
 *
 * @note Invalidates all iterators.
 *
 *    @param ptr_array Array being manipulated.
 *    @param element Element being pushed to the back.
 */
#define array_push_back(ptr_array, element) \
   do array_grow(ptr_array) = element; while (0)
/**
 * @brief Erases elements in interval [first, last).
 *
 * @note Invalidates all iterators.
 *
 *    @param ptr_array Array being manipulated.
 *    @param first First iterator to erase.
 *    @param last Last iterator in erase section but is not erased.
 */
#define array_erase(ptr_array, first, last) \
      (_array_erase_helper((void **)(ptr_array), sizeof((ptr_array)[0][0]), (void *)(first), (void *)(last)))
/**
 * @brief Shrinks memory to fit only `size' elements.
 *
 * @note Invalidates all iterators.
 *
 *    @param ptr_array Array being manipulated.
 */
#define array_shrink(ptr_array) \
      (_array_shrink_helper((void **)(ptr_array), sizeof((ptr_array)[0][0]))) // NOLINT
/**
 * @brief Frees memory allocated and sets array to NULL.
 *
 * @note Invalidates all iterators.
 *
 *    @param ptr_array Array being manipulated.
 */
#define array_free(ptr_array) \
      _array_free_helper((void *)(ptr_array))

/**
 * @brief Returns number of elements in the array.
 * \warning macro, may evaluate argument twice.
 *
 *    @param array Array being manipulated.
 *    @return The size of the array (number of elements).
 */
ALWAYS_INLINE static inline int array_size(const void *array)
{
   const _private_container *c1 = array;

   if (c1 == NULL)
      return 0;

#if DEBUG_ARRAYS
   assert("Sentinel not found. Use array_create() to create the array." && (c1[-1]._sentinel == ARRAY_SENTINEL));
#endif /* DEBUG_ARRAYS */

   return c1[-1]._size;
}
/**
 * @brief Returns number of elements reserved.
 *
 *    @param array Array being manipulated.
 *    @return The size of the array (memory usage).
 */
#define array_reserved(array) (_array_private_container(array)->_reserved)
/**
 * @brief Returns a pointer to the beginning of the reserved memory space.
 *
 *    @param array Array being manipulated.
 *    @return Beginning of memory space.
 */
#define array_begin(array) (array)
/**
 * @brief Returns a pointer to the end of the reserved memory space.
 * \warning macro, may evaluate argument twice.
 *
 *    @param array Array being manipulated.
 *    @return End of memory space.
 */
#define array_end(array) ((array) + array_size(array))
/**
 * @brief Returns the first element in the array.
 *
 *    @param ptr_array Array being manipulated.
 *    @return The first element in the array.
 */
#define array_front(ptr_array) (*array_begin(ptr_array))
/**
 * @brief Returns the last element in the array.
 *
 *    @param ptr_array Array being manipulated.
 *    @return The last element in the array.
 */
#define array_back(ptr_array) (*(array_end(ptr_array) - 1))
/** @brief Returns a shallow copy of the input array.  */
#define array_copy(basic_type, ptr_array) \
      ((basic_type *)(_array_copy_helper(sizeof(basic_type), (void *)(ptr_array))))
