pub mod atomicfloat;

/// Useful to sort by a reference to a String, which you can't do with sort_by_key due to
/// lifetimes.
pub fn sort_by_key_ref<T, F, K>(a: &mut [T], key: F)
where
   F: Fn(&T) -> &K,
   K: ?Sized + Ord,
{
   a.sort_by(|x, y| key(x).cmp(key(y)));
}

/// Useful to binary search by a reference to a String, which you can't do with
/// binary_search_by_key due to lifetimes.
pub fn binary_search_by_key_ref<T, F, K>(a: &[T], b: &K, key: F) -> Result<usize, usize>
where
   F: Fn(&T) -> &K,
   K: ?Sized + Ord,
{
   a.binary_search_by(|x| key(x).cmp(b))
}

use std::sync::atomic::{AtomicU32, Ordering};
/// Small wrapper for atomic f32 operations.
pub struct AtomicF32 {
   storage: AtomicU32,
}
impl AtomicF32 {
   pub const fn new(value: f32) -> Self {
      let as_u32 = value.to_bits();
      Self {
         storage: AtomicU32::new(as_u32),
      }
   }
   pub fn store(&self, value: f32, ordering: Ordering) {
      let as_u32 = value.to_bits();
      self.storage.store(as_u32, ordering)
   }
   pub fn load(&self, ordering: Ordering) -> f32 {
      let as_u32 = self.storage.load(ordering);
      f32::from_bits(as_u32)
   }
}

/// Slot map extension
pub trait ReferenceC {
   fn slot(&self) -> usize
   where
      Self: Sized;
   fn as_ffi(&self) -> i64
   where
      Self: Sized;
}
impl<T: slotmap::Key> ReferenceC for T {
   fn slot(&self) -> usize
   where
      Self: Sized,
   {
      // TODO this is not very safe and probably a bad idea
      (self.data().as_ffi() & 0xffff_ffff) as usize - 1
   }

   fn as_ffi(&self) -> i64
   where
      Self: Sized,
   {
      self.data().as_ffi() as i64
   }

   //fn from_ffi(value: i64) -> Self where Self: Sized {
   //   Self(KeyData::from_ffi(value as u64))
   //}
}
