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
unsafe impl<T: Send> Send for Array<T> {}
unsafe impl<T: Sync> Sync for Array<T> {}

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
unsafe impl Send for ArrayCString {}
unsafe impl Sync for ArrayCString {}
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

fn vec_into_raw_parts<T>(vec: Vec<T>) -> (*mut T, usize, usize) {
   // TODO just use into_raw_parts when we can use Rust 1.93 or later
   let mut me = std::mem::ManuallyDrop::new(vec);
   (me.as_mut_ptr(), me.len(), me.capacity())
}

fn vec_to_bytes<T>(vec: Vec<T>) -> Vec<u8> {
   let (ptr, len, cap) = vec_into_raw_parts(vec);
   //let (ptr, len, cap) = Vec::into_raw_parts(vec);
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

      let (ptr, len, capacity) = vec_into_raw_parts(vec);
      //let (ptr, len, capacity) = vec.into_raw_parts();
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
      (self.data, self.len, self.capacity) = vec_into_raw_parts(vec);
      //(self.data, self.len, self.capacity) = vec.into_raw_parts();
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

   let (data, len, capacity) = vec_into_raw_parts(src.to_vec());
   //let (data, len, capacity) = src.to_vec().into_raw_parts();
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
pub extern "C" fn _array_shrink_helper(array: *mut *mut c_void) {
   let data = unsafe { *array as *mut u8 };
   if data.is_null() {
      return;
   }
   let mut header = unsafe { header_from_data_mut(data) }.clone();
   // Rebuild Vec<u8>
   let mut vec = unsafe { header.rebuild_vec() };
   vec.shrink_to_fit();
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
   use rand::RngExt;
   use rand::distr::StandardUniform;
   use std::ffi::c_void;

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

   fn array_copy<T>(array: *mut T) -> *mut T {
      _array_copy_helper(array as *mut c_void) as *mut T
   }

   fn array_shrink<T>(array: &mut *mut T) {
      _array_shrink_helper(array as *mut *mut T as *mut *mut c_void);
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

   fn gen_vec<T>(n: usize) -> Vec<T>
   where
      StandardUniform: rand::distr::Distribution<T>,
   {
      let mut rng = rand::rng();
      let mut v: Vec<T> = Vec::new();
      for _ in 0..n {
         v.push(rng.random::<T>());
      }
      v
   }

   #[test]
   fn slice_rust_equality() {
      let v = gen_vec::<i32>(1000);
      let arr = Array::new(v.clone());
      let ptr = arr.as_ptr();
      let slice = unsafe { array_as_slice(ptr) };
      assert_eq!(slice, v.as_slice());
   }

   #[test]
   fn slice_equality() {
      let v = gen_vec::<i32>(1000);
      let ptr = Array::new(v.clone()).into_ptr();
      let slice = unsafe { array_as_slice(ptr) };
      assert_eq!(slice, v.as_slice());
      unsafe { _array_free_helper(ptr as *mut c_void) };
   }

   #[test]
   fn create() {
      let arr = _array_create_helper(size_of::<i32>(), 4);
      assert!(!arr.is_null());
      assert_eq!(_array_size_helper(arr), 0);
      unsafe { _array_free_helper(arr) };
   }

   #[test]
   fn create_cacity() {
      let arr = _array_create_helper(size_of::<i32>(), 8);
      assert_eq!(_array_reserved_helper(arr), 8);
      unsafe { _array_free_helper(arr) };
   }

   #[test]
   fn grow_write() {
      let mut arr = array_create::<i32>();
      unsafe { *array_grow(&mut arr) = 42 };
      unsafe { *array_grow(&mut arr) = 9 };
      unsafe { *array_grow(&mut arr) = 27 };
      assert_eq!(array_size(arr), 3);
      let slice = unsafe { array_as_slice(arr) };
      assert_eq!(slice, &[42, 9, 27]);
      array_free(arr);
   }

   #[test]
   fn grow_is_zeroed() {
      let mut arr = array_create::<u64>();
      for i in 1..1000 {
         let slot = array_grow(&mut arr);
         assert_eq!(unsafe { *slot }, 0);
         assert_eq!(array_size(arr), i);
      }
      array_free(arr);
   }

   #[test]
   fn erase() {
      let mut arr = array_create::<i32>();
      for i in 0..100 {
         unsafe { *array_grow(&mut arr) = i };
      }
      let start = arr.clone();
      let end = unsafe { arr.add(30) };
      array_erase(&mut arr, start, end);
      assert_eq!(array_size(arr), 70);
      assert_eq!(
         unsafe { array_as_slice(arr) },
         (30..100).collect::<Vec<_>>().as_slice()
      );
      let start = unsafe { arr.add(60) };
      let end = array_end(arr);
      array_erase(&mut arr, start, end);
      assert_eq!(array_size(arr), 60);
      assert_eq!(
         unsafe { array_as_slice(arr) },
         (30..90).collect::<Vec<_>>().as_slice()
      );
      // Out of range is no-op
      let start = array_end(arr);
      let end = unsafe { array_end(arr).add(60) };
      array_erase(&mut arr, start, end);
      assert_eq!(array_size(arr), 60);
      assert_eq!(
         unsafe { array_as_slice(arr) },
         (30..90).collect::<Vec<_>>().as_slice()
      );
      let start = array_begin(arr);
      let end = array_begin(arr);
      array_erase(&mut arr, start, end);
      assert_eq!(array_size(arr), 60);
      assert_eq!(
         unsafe { array_as_slice(arr) },
         (30..90).collect::<Vec<_>>().as_slice()
      );
      let start = array_begin(arr);
      let end = array_end(arr);

      array_erase(&mut arr, start, end);
      assert_eq!(array_size(arr), 0);
      array_free(arr);
   }

   #[test]
   fn null() {
      assert_eq!(array_size::<i32>(std::ptr::null_mut()), 0);
      assert_eq!(array_reserved::<i32>(std::ptr::null_mut()), 0);
      array_free::<i32>(std::ptr::null_mut());
   }

   #[test]
   fn resize() {
      let v = gen_vec::<i32>(1000);
      let mut ptr = Array::new(v.clone()).into_ptr();
      for i in 1..1000 {
         array_resize(&mut ptr, 1000 - i);
         assert_eq!(array_size(ptr), (1000 - i) as i32);
      }
      for i in 1..1000 {
         array_resize(&mut ptr, i);
         assert_eq!(array_size(ptr), i as i32);
         //assert_eq!( unsafe{ *array_end(ptr).sub(1) }, 0 );
      }
      array_free(ptr);
   }

   #[test]
   fn copy() {
      let mut arr = array_create::<i32>();
      for i in 0..4i32 {
         unsafe { *array_grow(&mut arr) = i * 3 };
      }
      let copy = array_copy(arr);
      assert_eq!(array_size(copy), 4);
      let orig = unsafe { array_as_slice(arr) };
      let duped = unsafe { array_as_slice_mut(copy) };
      assert_eq!(orig, duped);
      duped[1] = 42;
      assert!(orig[1] != duped[1]);
      array_free(arr);
      array_free(copy);
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
      assert_eq!(array_size(array), array_reserved(array));

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
      assert_eq!(array_size(array), array_reserved(array));

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
      assert_eq!(array_reserved(array), 0);
   }
}
