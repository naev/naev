use std::sync::atomic::{AtomicU32, AtomicU64, Ordering};

pub struct AtomicF64 {
    storage: AtomicU64,
}
impl AtomicF64 {
    pub const fn new(value: f64) -> Self {
        let as_u64 = value.to_bits();
        Self {
            storage: AtomicU64::new(as_u64),
        }
    }
    pub fn store(&self, value: f64, ordering: Ordering) {
        let as_u64 = value.to_bits();
        self.storage.store(as_u64, ordering)
    }
    pub fn load(&self, ordering: Ordering) -> f64 {
        let as_u64 = self.storage.load(ordering);
        f64::from_bits(as_u64)
    }
}

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
