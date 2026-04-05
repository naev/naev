#![allow(dead_code)]
use std::ffi::{CString, c_void};
use std::marker::PhantomData;
use std::ptr;

/// Size of the header
const PREFIX: usize = size_of::<Array<Dummy>>();

/// A flexible Array for C that can't just use Vec. Built around Vec::into_parts_raw.
#[derive(Debug, Clone)]
#[repr(C)]
pub struct Array<T> {
   element_size: usize,
   len: usize,      // in bytes
   capacity: usize, // in bytes
   data: *mut u8,   // Vec buffer
   _marker: PhantomData<T>,
}
// TODO this should be conditioned on T being Send or Sync
unsafe impl<T> Send for Array<T> {}
unsafe impl<T> Sync for Array<T> {}

impl<T> Default for Array<T> {
   fn default() -> Self {
      Array::new(Vec::new())
   }
}

/// Dummy struct for the C API
#[derive(Debug, Clone)]
struct Dummy;
type ArrayC = Array<Dummy>;

/// Wrapper to make it easier to deal with CString arrays
#[derive(Default, Debug)]
pub struct ArrayCString(Array<*mut i8>);
impl ArrayCString {
   pub fn new(vec: &[String]) -> Self {
      let data: Vec<_> = vec
         .iter()
         .map(|s| CString::new(s.as_str()).unwrap().into_raw())
         .collect();
      ArrayCString(Array::new(data))
   }

   pub fn as_ptr(&self) -> *mut *const i8 {
      self.0.as_ptr() as *mut *const i8
   }
}

fn vec_to_bytes<T>(vec: Vec<T>) -> Vec<u8> {
   let (ptr, len, cap) = Vec::into_raw_parts(vec);
   let len_bytes = len * std::mem::size_of::<T>();
   let cap_bytes = cap * std::mem::size_of::<T>();
   unsafe { Vec::from_raw_parts(ptr as *mut u8, len_bytes, cap_bytes) }
}

impl<T> Array<T> {
   pub fn new(vec: Vec<T>) -> Array<T> {
      let element_size = size_of::<T>();

      let mut vec = vec_to_bytes(vec);
      let length = vec.len();

      vec.reserve(PREFIX);
      unsafe {
         // Move existing data forward
         std::ptr::copy(vec.as_ptr(), vec.as_mut_ptr().add(PREFIX), length);
         vec.set_len(PREFIX + length);
      }

      let (ptr, len, capacity) = vec.into_raw_parts();
      let mut arr = Array {
         element_size,
         len,
         capacity,
         data: ptr,
         _marker: PhantomData,
      };

      // This will stamp the metadata
      unsafe { arr.write_header() };
      arr
   }

   /*
   pub fn as_slice( &self ) -> &[T] {
      unsafe {
         std::slice::from_raw_parts(
            self.data as *const T,
            self.len,
         )
      }
   }
   */

   pub fn as_ptr(&self) -> *mut T {
      unsafe { self.data.add(PREFIX) as *mut T }
   }

   pub fn into_ptr(self) -> *mut T {
      unsafe { self.data.add(PREFIX) as *mut T }
   }

   unsafe fn rebuild_vec(&mut self) -> Vec<u8> {
      unsafe { Vec::from_raw_parts(self.data, self.len, self.capacity) }
   }

   // Store Vec back into header
   fn store_vec(&mut self, vec: Vec<u8>) {
      (self.data, self.len, self.capacity) = vec.into_raw_parts();
   }

   // Stamp the header if, say, it's from a different data source like owned by Rust
   unsafe fn write_header(&mut self) {
      unsafe {
         ptr::copy_nonoverlapping(self as *mut Self as *mut u8, self.data, PREFIX);
      }
   }
}

// Get header from data pointer
unsafe fn header_from_data(data: *mut u8) -> &'static ArrayC {
   // Header is stored just before the data
   unsafe {
      let header_ptr_ptr = data.sub(PREFIX) as *mut ArrayC;
      let header = &*header_ptr_ptr;
      assert_eq!(header_ptr_ptr as *mut u8, header.data);
      header
   }
}

// Get header from data pointer
unsafe fn header_from_data_mut(data: *mut u8) -> &'static mut ArrayC {
   // We store header pointer just before the data pointer
   unsafe {
      let header_ptr_ptr = data.sub(PREFIX) as *mut ArrayC;
      let header = &mut *header_ptr_ptr;
      assert_eq!(header_ptr_ptr as *mut u8, header.data);
      header
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn _array_create_helper(
   element_size: usize,
   initial_capacity: usize,
) -> *mut c_void {
   let cap = initial_capacity.max(1) * element_size;
   let mut vec = Vec::<u8>::with_capacity(PREFIX + cap);
   unsafe {
      vec.set_len(PREFIX);
   }
   let (data, len, capacity) = vec.into_raw_parts();
   let mut arr = Array {
      element_size,
      len,
      capacity,
      data,
      _marker: PhantomData,
   };
   unsafe { arr.write_header() };
   arr.into_ptr() as *mut c_void
}

#[unsafe(no_mangle)]
pub extern "C" fn _array_grow_helper(array: *mut *mut c_void) -> *mut c_void {
   let data = unsafe { *array as *mut u8 };
   if data.is_null() {
      return std::ptr::null_mut();
   }
   let mut header = unsafe { header_from_data_mut(data) }.clone();

   let mut vec = unsafe { header.rebuild_vec() };

   let old_len = vec.len();
   vec.resize(old_len + header.element_size, 0);
   header.store_vec(vec);
   unsafe {
      header.write_header();
      *array = header.as_ptr() as *mut c_void;
   }
   unsafe { header.data.add(old_len) as *mut c_void }
}

#[unsafe(no_mangle)]
pub extern "C" fn _array_erase_helper(
   array: *mut *mut c_void,
   start: *mut c_void,
   end: *mut c_void,
) {
   let data = unsafe { *array as *mut u8 };
   if data.is_null() {
      return;
   }
   let mut header = unsafe { header_from_data_mut(data) }.clone();

   let base = header.data as usize;
   let s = start as usize - base;
   let e = end as usize - base;

   if s >= e || e > header.len {
      return;
   }

   let mut vec = unsafe { header.rebuild_vec() };
   vec.drain(s..e);
   header.store_vec(vec);

   unsafe {
      header.write_header();
      *array = header.as_ptr() as *mut c_void;
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn _array_size_helper(array: *mut c_void) -> i32 {
   if array.is_null() {
      return 0;
   }
   let header = unsafe { header_from_data(array as *mut u8) };
   ((header.len - PREFIX) / header.element_size) as i32
}

#[unsafe(no_mangle)]
pub extern "C" fn _array_reserved_helper(array: *mut c_void) -> i32 {
   if array.is_null() {
      return 0;
   }
   let header = unsafe { header_from_data(array as *mut u8) };
   ((header.capacity - PREFIX) / header.element_size) as i32
}

#[unsafe(no_mangle)]
pub extern "C" fn _array_copy_helper(array: *mut c_void) -> *mut c_void {
   if array.is_null() {
      return std::ptr::null_mut();
   }
   let header = unsafe { header_from_data_mut(array as *mut u8) };
   let src = unsafe { std::slice::from_raw_parts(header.data, header.len) };

   let (data, len, capacity) = src.to_vec().into_raw_parts();
   let mut new = ArrayC {
      element_size: header.element_size,
      len,
      capacity,
      data,
      _marker: PhantomData,
   };
   unsafe { new.write_header() };
   new.as_ptr() as *mut c_void
}

#[unsafe(no_mangle)]
pub extern "C" fn _array_resize_helper(array: *mut *mut c_void, new_size: usize) {
   let data = unsafe { *array as *mut u8 };
   if data.is_null() {
      return;
   }
   let mut header = unsafe { header_from_data_mut(data) }.clone();
   // Rebuild Vec<u8>
   let mut vec = unsafe { header.rebuild_vec() };
   let new_len_bytes = PREFIX + new_size * header.element_size;
   if new_len_bytes > vec.len() {
      // Grow → zero-initialize new memory
      vec.resize(new_len_bytes, 0);
   } else {
      // Shrink
      vec.truncate(new_len_bytes);
   }

   header.store_vec(vec);

   // Update pointer for C side
   unsafe {
      header.write_header();
      *array = header.as_ptr() as *mut c_void;
   }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn _array_free_helper(array: *mut c_void) {
   if array.is_null() {
      return;
   }
   unsafe {
      let header = header_from_data_mut(array as *mut u8);
      let _vec = header.rebuild_vec(); // dropped here
   }
}

pub unsafe fn array_as_slice<T>(array: *mut T) -> &'static [T] {
   if array.is_null() {
      return &[];
   }
   unsafe {
      let header = header_from_data(array as *mut u8);
      std::slice::from_raw_parts(
         array as *const T,
         (header.len - PREFIX) / header.element_size,
      )
   }
}

pub unsafe fn array_as_slice_mut<T>(array: *mut T) -> &'static mut [T] {
   if array.is_null() {
      return &mut [];
   }
   unsafe {
      let header = header_from_data_mut(array as *mut u8);
      std::slice::from_raw_parts_mut(array as *mut T, (header.len - PREFIX) / header.element_size)
   }
}

#[cfg(test)]
mod tests {
   use super::*;
   use std::ffi::c_void;

   /// Cast a raw C-style array pointer to a typed slice for easy assertions.
   unsafe fn as_slice<T>(ptr: *mut c_void, len: usize) -> &'static [T] {
      let slice = unsafe { array_as_slice::<T>(ptr as *mut T) };
      assert_eq!(slice.len(), len);
      slice
   }

   fn array_create<T>() -> *mut T {
      _array_create_helper(std::mem::size_of::<T>(), 1) as *mut T
   }

   fn array_create_size<T>(new_size: usize) -> *mut T {
      _array_create_helper(std::mem::size_of::<T>(), new_size) as *mut T
   }

   fn array_resize<T>(array: &mut *mut T, new_size: usize) {
      _array_resize_helper(array as *mut *mut T as *mut *mut c_void, new_size);
   }

   fn array_grow<T>(array: &mut *mut T) -> *mut T {
      _array_grow_helper(array as *mut *mut T as *mut *mut c_void) as *mut T
   }

   fn array_push_back<T>(array: &mut *mut T, value: T) {
      let ptr = array_grow(array) as *mut T;
      unsafe { *ptr = value };
   }

   fn array_erase<T>(array: &mut *mut T, first: *mut T, last: *mut T) {
      _array_erase_helper(
         array as *mut *mut T as *mut *mut c_void,
         first as *mut c_void,
         last as *mut c_void,
      );
   }

   fn array_shrink<T>(array: &mut *mut T) {
      let len = array_size(*array) as usize;
      _array_resize_helper(array as *mut *mut T as *mut *mut c_void, len);
   }

   fn array_free<T>(array: *mut T) {
      unsafe { _array_free_helper(array as *mut c_void) };
   }

   fn array_size<T>(array: *mut T) -> i32 {
      _array_size_helper(array as *mut c_void)
   }

   fn array_reserved<T>(array: *mut T) -> i32 {
      _array_reserved_helper(array as *mut c_void)
   }

   fn array_begin<T>(array: *mut T) -> *mut T {
      array
   }

   fn array_end<T>(array: *mut T) -> *mut T {
      unsafe { array.add(array_size(array) as usize) }
   }

   #[test]
   fn rust_new_preserves_elements() {
      let v = vec![10u32, 20, 30];
      let arr = Array::new(v);
      let ptr = arr.as_ptr();
      let slice = unsafe { array_as_slice(ptr) };
      assert_eq!(slice, &[10u32, 20, 30]);
   }

   #[test]
   fn rust_default_is_empty() {
      let arr = Array::<i32>::default();
      // into_ptr would be needed to call the C helpers; just verify it builds
      let _ = arr;
   }

   #[test]
   fn rust_into_ptr_roundtrip() {
      let v = vec![1i32, 2, 3];
      let ptr = Array::new(v).into_ptr();
      // header_from_data should now work on this pointer
      let slice = unsafe { array_as_slice(ptr) };
      assert_eq!(slice, &[1i32, 2, 3]);
      unsafe { _array_free_helper(ptr as *mut c_void) };
   }

   #[test]
   fn create_size_is_zero() {
      let arr = _array_create_helper(size_of::<i32>(), 4);
      assert_eq!(_array_size_helper(arr), 0);
      unsafe { _array_free_helper(arr) };
   }

   #[test]
   fn create_reserved_matches_capacity() {
      let arr = _array_create_helper(size_of::<i32>(), 8);
      assert_eq!(_array_reserved_helper(arr), 8);
      unsafe { _array_free_helper(arr) };
   }

   #[test]
   fn create_zero_capacity_clamps_to_one() {
      // initial_capacity of 0 should not panic; the code uses .max(1)
      let arr = _array_create_helper(size_of::<u8>(), 0);
      assert!(_array_reserved_helper(arr) >= 1);
      unsafe { _array_free_helper(arr) };
   }

   #[test]
   fn create_returns_non_null() {
      let arr = _array_create_helper(size_of::<u64>(), 4);
      assert!(!arr.is_null());
      unsafe { _array_free_helper(arr) };
   }

   #[test]
   fn grow_increments_size() {
      let mut arr = _array_create_helper(size_of::<i32>(), 2);
      unsafe { *(_array_grow_helper(&mut arr) as *mut i32) = 42 };
      assert_eq!(_array_size_helper(arr), 1);
      unsafe { _array_free_helper(arr) };
   }

   #[test]
   fn grow_returned_pointer_is_writable() {
      let mut arr = _array_create_helper(size_of::<u32>(), 4);
      let slot = _array_grow_helper(&mut arr) as *mut u32;
      unsafe { *slot = 0xDEADBEEF };
      let slice = unsafe { as_slice::<u32>(arr, 1) };
      assert_eq!(slice[0], 0xDEADBEEF);
      unsafe { _array_free_helper(arr) };
   }

   #[test]
   fn grow_multiple_elements_ordered() {
      let mut arr = _array_create_helper(size_of::<i32>(), 4);
      for i in 0..4i32 {
         unsafe { *(_array_grow_helper(&mut arr) as *mut i32) = i * 10 };
      }
      let slice = unsafe { as_slice::<i32>(arr, 4) };
      assert_eq!(slice, &[0, 10, 20, 30]);
      unsafe { _array_free_helper(arr) };
   }

   #[test]
   fn grow_beyond_initial_capacity_reallocates() {
      let mut arr = _array_create_helper(size_of::<i32>(), 1);
      for i in 0..8i32 {
         unsafe { *(_array_grow_helper(&mut arr) as *mut i32) = i };
      }
      assert_eq!(_array_size_helper(arr), 8);
      let slice = unsafe { as_slice::<i32>(arr, 8) };
      for (i, &v) in slice.iter().enumerate() {
         assert_eq!(v, i as i32);
      }
      unsafe { _array_free_helper(arr) };
   }

   #[test]
   fn grow_new_slot_is_zeroed() {
      let mut arr = _array_create_helper(size_of::<u64>(), 4);
      let slot = _array_grow_helper(&mut arr) as *mut u64;
      assert_eq!(unsafe { *slot }, 0);
      unsafe { _array_free_helper(arr) };
   }

   #[test]
   fn erase_middle_element() {
      let mut arr = _array_create_helper(size_of::<i32>(), 4);
      for i in 0..4i32 {
         unsafe { *(_array_grow_helper(&mut arr) as *mut i32) = i };
      }
      // erase first element
      let start = unsafe { (arr as *mut i32).add(1) as *mut c_void };
      let end = unsafe { (arr as *mut i32).add(2) as *mut c_void };
      _array_erase_helper(&mut arr, start, end);
      assert_eq!(_array_size_helper(arr), 3);
      let slice = unsafe { as_slice::<i32>(arr, 3) };
      assert_eq!(slice, &[0, 2, 3]);
      unsafe { _array_free_helper(arr) };
   }

   #[test]
   fn erase_all_elements() {
      let mut arr = _array_create_helper(size_of::<i32>(), 4);
      for i in 0..4i32 {
         unsafe { *(_array_grow_helper(&mut arr) as *mut i32) = i };
      }
      let start = arr as *mut c_void;
      let end = unsafe { (arr as *mut i32).add(4) as *mut c_void };
      _array_erase_helper(&mut arr, start, end);
      assert_eq!(_array_size_helper(arr), 0);
      unsafe { _array_free_helper(arr) };
   }

   #[test]
   fn erase_empty_range_is_noop() {
      let mut arr = _array_create_helper(size_of::<i32>(), 4);
      for i in 0..3i32 {
         unsafe { *(_array_grow_helper(&mut arr) as *mut i32) = i };
      }
      let p = unsafe { (arr as *mut i32).add(1) as *mut c_void };
      _array_erase_helper(&mut arr, p, p);
      assert_eq!(_array_size_helper(arr), 3);
      unsafe { _array_free_helper(arr) };
   }

   #[test]
   fn erase_out_of_bounds_is_noop() {
      let mut arr = _array_create_helper(size_of::<i32>(), 4);
      for i in 0..3i32 {
         unsafe { *(_array_grow_helper(&mut arr) as *mut i32) = i };
      }
      let start = arr as *mut c_void;
      let end = unsafe { (arr as *mut i32).add(10) as *mut c_void };
      _array_erase_helper(&mut arr, start, end);
      assert_eq!(_array_size_helper(arr), 3);
      unsafe { _array_free_helper(arr) };
   }

   #[test]
   fn size_null_returns_zero() {
      assert_eq!(_array_size_helper(std::ptr::null_mut()), 0);
   }

   #[test]
   fn reserved_null_returns_zero() {
      assert_eq!(_array_reserved_helper(std::ptr::null_mut()), 0);
   }

   #[test]
   fn size_tracks_grow_and_erase() {
      let mut arr = _array_create_helper(size_of::<u8>(), 8);
      for _ in 0..5u8 {
         _array_grow_helper(&mut arr);
      }
      assert_eq!(_array_size_helper(arr), 5);

      let start = arr as *mut c_void;
      let end = unsafe { (arr as *mut u8).add(2) as *mut c_void };
      _array_erase_helper(&mut arr, start, end);
      assert_eq!(_array_size_helper(arr), 3);
      unsafe { _array_free_helper(arr) };
   }

   #[test]
   fn resize_grow_zero_initializes() {
      let mut arr = _array_create_helper(size_of::<i32>(), 2);
      assert_eq!(_array_reserved_helper(arr), 2);
      _array_resize_helper(&mut arr, 4);
      assert_eq!(_array_size_helper(arr), 4);
      let slice = unsafe { as_slice::<i32>(arr, 4) };
      assert!(slice.iter().all(|&v| v == 0));
      unsafe { _array_free_helper(arr) };
   }

   #[test]
   fn resize_shrink_truncates() {
      let mut arr = _array_create_helper(size_of::<i32>(), 8);
      for i in 0..6i32 {
         unsafe { *(_array_grow_helper(&mut arr) as *mut i32) = i };
      }
      _array_resize_helper(&mut arr, 3);
      assert_eq!(_array_size_helper(arr), 3);
      let slice = unsafe { as_slice::<i32>(arr, 3) };
      assert_eq!(slice, &[0, 1, 2]);
      unsafe { _array_free_helper(arr) };
   }

   #[test]
   fn resize_to_zero() {
      let mut arr = _array_create_helper(size_of::<i32>(), 4);
      for i in 0..4i32 {
         unsafe { *(_array_grow_helper(&mut arr) as *mut i32) = i };
      }
      _array_resize_helper(&mut arr, 0);
      assert_eq!(_array_size_helper(arr), 0);
      unsafe { _array_free_helper(arr) };
   }

   #[test]
   fn resize_null_array_is_noop() {
      _array_resize_helper(std::ptr::null_mut(), 4);
   }

   #[test]
   fn copy_produces_equal_elements() {
      let mut arr = _array_create_helper(size_of::<i32>(), 4);
      for i in 0..4i32 {
         unsafe { *(_array_grow_helper(&mut arr) as *mut i32) = i * 3 };
      }
      let copy = _array_copy_helper(arr);
      assert_eq!(_array_size_helper(copy), 4);
      let orig = unsafe { as_slice::<i32>(arr, 4) };
      let duped = unsafe { as_slice::<i32>(copy, 4) };
      assert_eq!(orig, duped);
      unsafe { _array_free_helper(arr) };
      unsafe { _array_free_helper(copy) };
   }

   #[test]
   fn copy_is_independent() {
      // Mutating the copy must not affect the original.
      let mut arr = _array_create_helper(size_of::<i32>(), 4);
      for i in 0..4i32 {
         unsafe { *(_array_grow_helper(&mut arr) as *mut i32) = i };
      }
      let copy = _array_copy_helper(arr);
      // Overwrite first element of copy.
      unsafe { *(copy as *mut i32) = 99 };
      let orig_slice = unsafe { as_slice::<i32>(arr, 4) };
      let copy_slice = unsafe { as_slice::<i32>(copy, 4) };
      assert_eq!(orig_slice[0], 0); // original unchanged
      assert_eq!(copy_slice[0], 99); // copy mutated
      unsafe { _array_free_helper(arr) };
      unsafe { _array_free_helper(copy) };
   }

   #[test]
   fn copy_of_empty_array() {
      let arr = _array_create_helper(size_of::<i32>(), 4);
      let copy = _array_copy_helper(arr);
      assert_eq!(_array_size_helper(copy), 0);
      unsafe { _array_free_helper(arr) };
      unsafe { _array_free_helper(copy) };
   }

   #[test]
   fn free_null_is_noop() {
      unsafe { _array_free_helper(std::ptr::null_mut()) };
   }

   #[test]
   fn as_slice_reflects_current_state() {
      let v = vec![10u32, 20, 30, 40];
      let ptr = Array::new(v).into_ptr();
      let slice = unsafe { array_as_slice(ptr) };
      assert_eq!(slice, &[10u32, 20, 30, 40]);
      unsafe { _array_free_helper(ptr as *mut c_void) };
   }

   #[test]
   fn as_slice_mut_allows_writes() {
      let v = vec![1i32, 2, 3];
      let ptr = Array::new(v).into_ptr();
      let slice = unsafe { array_as_slice_mut(ptr) };
      slice[1] = 99;
      let check = unsafe { array_as_slice(ptr) };
      assert_eq!(check, &[1i32, 99, 3]);
      unsafe { _array_free_helper(ptr as *mut c_void) };
   }

   #[test]
   fn cstring_array_as_ptr_non_null() {
      let strings = vec!["hello".to_string(), "world".to_string()];
      let arr = ArrayCString::new(&strings);
      assert!(!arr.as_ptr().is_null());
   }

   #[test]
   fn cstring_array_roundtrips_strings() {
      use std::ffi::CStr;
      let strings = vec!["foo".to_string(), "bar".to_string(), "baz".to_string()];
      let arr = ArrayCString::new(&strings);
      let ptr = arr.as_ptr();
      for (i, expected) in strings.iter().enumerate() {
         let c_ptr = unsafe { *ptr.add(i) };
         let got = unsafe { CStr::from_ptr(c_ptr).to_str().unwrap() };
         assert_eq!(got, expected.as_str());
      }
   }

   #[test]
   fn cstring_array_default_is_empty() {
      let arr = ArrayCString::default();
      let _ = arr.as_ptr();
   }

   #[test]
   fn large_element_size_u128() {
      let mut arr = _array_create_helper(size_of::<u128>(), 4);
      for i in 0..4u128 {
         unsafe { *(_array_grow_helper(&mut arr) as *mut u128) = u128::MAX - i };
      }
      assert_eq!(_array_size_helper(arr), 4);
      let slice = unsafe { as_slice::<u128>(arr, 4) };
      assert_eq!(slice[0], u128::MAX);
      assert_eq!(slice[3], u128::MAX - 3);
      unsafe { _array_free_helper(arr) };
   }

   #[test]
   fn stress_grow_and_erase_interleaved() {
      let mut arr = _array_create_helper(size_of::<i32>(), 2);
      // Fill to 10 elements.
      for i in 0..10i32 {
         unsafe { *(_array_grow_helper(&mut arr) as *mut i32) = i };
      }
      // Erase elements [3, 7).
      let start = unsafe { (arr as *mut i32).add(3) as *mut c_void };
      let end = unsafe { (arr as *mut i32).add(7) as *mut c_void };
      _array_erase_helper(&mut arr, start, end);
      assert_eq!(_array_size_helper(arr), 6);
      let slice = unsafe { as_slice::<i32>(arr, 6) };
      assert_eq!(slice, &[0, 1, 2, 7, 8, 9]);
      unsafe { _array_free_helper(arr) };
   }

   #[test]
   fn original_test() {
      // Original test taken from array.c.
      const SIZE: i32 = 100;

      let mut array = array_create::<i32>();

      for i in 0..SIZE {
         array_push_back(&mut array, i);
      }
      assert_eq!(array_size(array), SIZE);
      assert!(array_size(array) <= array_reserved(array));
      for i in 0..SIZE {
         unsafe {
            assert_eq!(*array.add(i as usize), i);
         }
      }

      /* erases second half */
      let begin = unsafe { array.add(SIZE as usize / 2) };
      let end = unsafe { array.add(SIZE as usize) };
      array_erase(&mut array, begin, end);
      assert_eq!(array_size(array), SIZE / 2);
      assert!(array_size(array) <= array_reserved(array));
      for i in 0..SIZE / 2 {
         unsafe {
            assert_eq!(*array.add(i as usize), i);
         }
      }

      /* shrinks */
      array_shrink(&mut array);
      //assert_eq!(array_size(array), array_reserved(array));

      /* pushes back second half */
      for i in (SIZE / 2)..SIZE {
         array_push_back(&mut array, i);
      }
      assert_eq!(array_size(array), SIZE);
      assert!(array_size(array) <= array_reserved(array));
      for i in 0..SIZE {
         unsafe {
            assert_eq!(*array.add(i as usize), i);
         }
      }

      /* erases middle half */
      let begin = unsafe { array.add(SIZE as usize / 4) };
      let end = unsafe { array.add(3 * SIZE as usize / 4) };
      array_erase(&mut array, begin, end);
      assert_eq!(array_size(array), SIZE / 2);
      assert!(array_size(array) <= array_reserved(array));
      for i in 0..(SIZE / 4) {
         unsafe {
            assert_eq!(*array.add(i as usize), i);
         }
      }
      for i in (SIZE / 4)..(SIZE / 2) {
         unsafe {
            assert_eq!(*array.add(i as usize), i + SIZE / 2);
         }
      }

      /* shrinks */
      array_shrink(&mut array);
      //assert_eq!(array_size(array), array_reserved(array));

      /* erases one element */
      let begin = array;
      let end = unsafe { array.add(1) };
      array_erase(&mut array, begin, end);
      assert_eq!(array_size(array), SIZE / 2 - 1);
      for i in 1..SIZE / 4 {
         unsafe {
            assert_eq!(*array.add(i as usize - 1), i);
         }
      }
      for i in (SIZE / 4)..(SIZE / 2) {
         unsafe {
            assert_eq!(*array.add(i as usize - 1), i + SIZE / 2);
         }
      }

      /* erases no elements */
      let begin = array;
      let end = array;
      array_erase(&mut array, begin, end);
      let begin = unsafe { array.add(array_size(array) as usize) };
      let end = unsafe { array.add(array_size(array) as usize) };
      array_erase(&mut array, begin, end);
      assert_eq!(array_size(array), SIZE / 2 - 1);
      for i in 1..SIZE / 4 {
         unsafe {
            assert_eq!(*array.add(i as usize - 1), i);
         }
      }
      for i in (SIZE / 4)..(SIZE / 2) {
         unsafe {
            assert_eq!(*array.add(i as usize - 1), i + SIZE / 2);
         }
      }

      /* erases all elements */
      let begin = array_begin(array);
      let end = array_end(array);
      array_erase(&mut array, begin, end);
      assert_eq!(array_size(array), 0);

      /* shrinks */
      array_shrink(&mut array);
      assert_eq!(array_size(array), 0);
      //assert_eq!(array_reserved(array), 1);
   }
}
