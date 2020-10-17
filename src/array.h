/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file array.h
 *
 * @brief Provides macros to work with dynamic arrays.
 *
 * @note Except were noted, macros do not have side effects from
 * expations.
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
 * array_shrink( my_array );
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

#ifndef ARRAY_H
#  define ARRAY_H

#include <stddef.h>
#include <assert.h>
#include <stdint.h>
#ifdef HAVE_STDALIGN_H
#include <stdalign.h>
#endif /* HAVE_STDALIGN_H */

#ifdef DEBUGGING
#define _ARRAY_SENTINEL ((int)0xbabecafe) /**< Badass sentinel. */
#endif

/**
 * @brief Private container type for the arrays.
 */
typedef struct {
#ifdef DEBUGGING
   int _sentinel;         /**< Sentinel for when debugging. */
#endif
   int _reserved;         /**< Number of elements reserved */
   int _size;             /**< Number of elements in the array */
   /* The following check is fairly nasty and is here to handle cases
    * when being compiled with too old versions of gcc. Note that this
    * does lead to undefined behaviour, but at the current time it is
    * necessary to compile for Steam. */
#ifdef HAVE_STDALIGN_H
   char alignas(max_align_t) _array[0];  /**< Begin of the array */
#else /* HAVE_STDALIGN_H */
   char _array[0]; /* Undefined behaviour that seems to "work" */
#endif /* HAVE_STDALIGN_H */
} _private_container;


void *_array_create_helper(size_t e_size, int initial_size);
void *_array_grow_helper(void **a, size_t e_size);
void _array_resize_helper(void **a, size_t e_size, int new_size);
void _array_erase_helper(void **a, size_t e_size, void *first, void *last);
void _array_shrink_helper(void **a, size_t e_size);
void _array_free_helper(void *a);

/**
 * @brief Gets the container of an array.
 *
 *    @param a Array to get container of.
 *    @return The container of the array a.
 */
__inline__ static _private_container *_array_private_container(void *a)
{
   assert("NULL array!" && (a != NULL));

   _private_container *c = (_private_container *)a - 1;

#ifdef DEBUGGING
   assert("Sentinel not found. Use array_create() to create the array." && (c->_sentinel == _ARRAY_SENTINEL));
#endif

   return c;
}

/**
 * @brief Gets the end of the array.
 *
 *    @param a Array to get end of.
 *    @param e_size Size of array members.
 *    @return The end of the array a.
 */
__inline__ static void *_array_end_helper(void *a, size_t e_size)
{
   _private_container *c = _array_private_container(a);
   return c->_array + c->_size * e_size;
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
      (*(__typeof__((ptr_array)[0]))_array_grow_helper((void **)(ptr_array), sizeof((ptr_array)[0][0])))
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
      (_array_shrink_helper((void **)(ptr_array), sizeof((ptr_array)[0][0])))
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
 *
 *    @param ptr_array Array being manipulated.
 *    @return The size of the array (number of elements).
 */
#define array_size(array) (_array_private_container(array)->_size)
/**
 * @brief Returns number of elements reserved.
 *
 *    @param ptr_array Array being manipulated.
 *    @return The size of the array (memory usage).
 */
#define array_reserved(array) (_array_private_container(array)->_reserved)
/**
 * @brief Returns a pointer to the beginning of the reserved memory space.
 *
 *    @param ptr_array Array being manipulated.
 *    @return Beginning of memory space.
 */
#define array_begin(array) (array)
/**
 * @brief Returns a pointer to the end of the reserved memory space.
 *
 *    @param ptr_array Array being manipulated.
 *    @return End of memory space.
 */
#define array_end(array) ((__typeof__(array))_array_end_helper((array), sizeof((array)[0])))
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


#endif /* ARRAY_H */


