#![allow(dead_code)]
use sdl3 as sdl;
use std::cell::UnsafeCell;
use std::ffi::CStr;
use std::ops::{Deref, DerefMut};
use std::sync::{LockResult, TryLockError, TryLockResult};

pub struct Lock(*mut sdl::sys::mutex::SDL_Mutex);
impl Lock {
    pub fn new() -> Self {
        let lock = unsafe { sdl::sys::mutex::SDL_CreateMutex() };
        if lock.is_null() {
            let e = unsafe { CStr::from_ptr(sdl::sys::error::SDL_GetError()) };
            //panic!(String::from(e.to_str().unwrap()));
            panic!("{}", e.to_str().unwrap());
        }
        Self(lock)
    }
    pub fn lock(&self) {
        unsafe { sdl::sys::mutex::SDL_LockMutex(self.0) }
    }
    pub fn unlock(&self) {
        unsafe { sdl::sys::mutex::SDL_UnlockMutex(self.0) }
    }
    pub fn try_lock(&self) -> bool {
        unsafe { sdl::sys::mutex::SDL_TryLockMutex(self.0) }
    }
}
impl Drop for Lock {
    fn drop(&mut self) {
        unsafe { sdl::sys::mutex::SDL_DestroyMutex(self.0) }
    }
}

pub struct Mutex<T: ?Sized> {
    inner: Lock,
    data: UnsafeCell<T>,
}
unsafe impl<T: ?Sized + Send> Send for Mutex<T> {}
unsafe impl<T: ?Sized + Send> Sync for Mutex<T> {}

pub struct MutexGuard<'a, T: ?Sized + 'a> {
    lock: &'a Mutex<T>,
}
//impl<T: ?Sized> !Send for MutexGuard<'_, T> {}
unsafe impl<T: ?Sized + Sync> Sync for MutexGuard<'_, T> {}
impl<'mutex, T: ?Sized> MutexGuard<'mutex, T> {
    unsafe fn new(lock: &'mutex Mutex<T>) -> LockResult<MutexGuard<'mutex, T>> {
        Ok(MutexGuard { lock })
    }
}
impl<T: ?Sized> Deref for MutexGuard<'_, T> {
    type Target = T;
    fn deref(&self) -> &T {
        unsafe { &*self.lock.data.get() }
    }
}
impl<T: ?Sized> DerefMut for MutexGuard<'_, T> {
    fn deref_mut(&mut self) -> &mut T {
        unsafe { &mut *self.lock.data.get() }
    }
}
impl<T: ?Sized> Drop for MutexGuard<'_, T> {
    #[inline]
    fn drop(&mut self) {
        self.lock.inner.unlock();
    }
}

impl<T> Mutex<T> {
    #[inline]
    pub fn new(t: T) -> Mutex<T> {
        Mutex {
            inner: Lock::new(),
            data: UnsafeCell::new(t),
        }
    }
}
impl<T: ?Sized> Mutex<T> {
    pub fn lock(&mut self) -> LockResult<MutexGuard<'_, T>> {
        unsafe {
            self.inner.lock();
            MutexGuard::new(self)
        }
    }
    pub fn try_lock(&self) -> TryLockResult<MutexGuard<'_, T>> {
        unsafe {
            if self.inner.try_lock() {
                Ok(MutexGuard::new(self)?)
            } else {
                Err(TryLockError::WouldBlock)
            }
        }
    }
}

#[test]
fn test_mutex() {
    use std::sync::Arc;
    use std::sync::Mutex;
    let mutex = Arc::new(Mutex::new(0_usize));
    let mut threads = Vec::new();
    for _ in 0..4 {
        let mutex_ref = mutex.clone();

        threads.push(std::thread::spawn(move || {
            for _ in 0..1_000_000 {
                let counter = &mut mutex_ref.lock().unwrap();
                **counter += 1;
            }
        }));
    }
    // Wait for all threads to finish
    for thread in threads {
        thread.join().unwrap();
    }
    assert_eq!(*mutex.lock().unwrap(), 4_000_000);
}
