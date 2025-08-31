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
