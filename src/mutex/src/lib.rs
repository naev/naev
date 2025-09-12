use sdl3 as sdl;
use std::cell::UnsafeCell;
use std::ffi::CStr;
use std::sync::{
    LockResult, PoisonError, TryLockError, TryLockResult, atomic::AtomicBool, atomic::Ordering,
};

struct Lock(*mut sdl::sys::mutex::SDL_Mutex);
impl Lock {
    fn new() -> Self {
        let lock = unsafe { sdl::sys::mutex::SDL_CreateMutex() };
        if lock.is_null() {
            let e = unsafe { CStr::from_ptr(sdl::sys::error::SDL_GetError()) };
            panic!("{}", e.to_str().unwrap());
        }
        Self(lock)
    }
    fn lock(&self) {
        unsafe { sdl::sys::mutex::SDL_LockMutex(self.0) }
    }
    fn unlock(&self) {
        unsafe { sdl::sys::mutex::SDL_UnlockMutex(self.0) }
    }
    fn try_lock(&self) -> bool {
        unsafe { sdl::sys::mutex::SDL_TryLockMutex(self.0) }
    }
}
impl Drop for Lock {
    fn drop(&mut self) {
        unsafe { sdl::sys::mutex::SDL_DestroyMutex(self.0) }
    }
}

pub struct MutexGuard<'a, T: ?Sized + 'a> {
    lock: &'a Mutex<T>,
    _marker: std::marker::PhantomData<*mut ()>,
}
//impl<T: ?Sized> !Send for MutexGuard<'_, T> {}
unsafe impl<T: ?Sized + Sync> Sync for MutexGuard<'_, T> {}
impl<'mutex, T: ?Sized> MutexGuard<'mutex, T> {
    unsafe fn new(lock: &'mutex Mutex<T>) -> LockResult<MutexGuard<'mutex, T>> {
        match lock
            .poison
            .compare_exchange(false, true, Ordering::Acquire, Ordering::Relaxed)
        {
            Ok(_) => Ok(MutexGuard {
                lock,
                _marker: std::marker::PhantomData::default(),
            }),
            Err(_) => Err(PoisonError::new(MutexGuard {
                lock,
                _marker: std::marker::PhantomData::default(),
            })),
        }
    }
}
impl<T: ?Sized> std::ops::Deref for MutexGuard<'_, T> {
    type Target = T;
    fn deref(&self) -> &T {
        unsafe { &*self.lock.data.get() }
    }
}
// Don't allow getting mutable references and force using RefCell so it doesn't violate Rust's no
// double borrowed muts rule.
/*
impl<T: ?Sized> std::ops::DerefMut for MutexGuard<'_, T> {
    fn deref_mut(&mut self) -> &mut T {
        unsafe { &mut *self.lock.data.get() }
    }
}
*/
impl<T: ?Sized> Drop for MutexGuard<'_, T> {
    #[inline]
    fn drop(&mut self) {
        self.lock.poison.store(false, Ordering::Relaxed);
        self.lock.inner.unlock();
    }
}

pub struct Mutex<T: ?Sized> {
    inner: Lock,
    poison: AtomicBool,
    data: UnsafeCell<T>,
}
unsafe impl<T: ?Sized + Send> Send for Mutex<T> {}
unsafe impl<T: ?Sized + Send> Sync for Mutex<T> {}
impl<T> Mutex<T> {
    #[inline]
    pub fn new(t: T) -> Mutex<T> {
        Mutex {
            inner: Lock::new(),
            poison: AtomicBool::new(false),
            data: UnsafeCell::new(t),
        }
    }
}
impl<T: ?Sized> Mutex<T> {
    pub fn lock(&self) -> LockResult<MutexGuard<'_, T>> {
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
impl<T: Default> Default for Mutex<T> {
    fn default() -> Mutex<T> {
        Mutex::new(Default::default())
    }
}

#[test]
fn test_mutex() {
    use std::sync::Arc;
    let mutex = Arc::new(Mutex::new(0_usize));
    let mut threads = Vec::new();
    for _ in 0..4 {
        let mutex_ref = mutex.clone();

        threads.push(std::thread::spawn(move || {
            for _ in 0..1_000_000 {
                let mut counter = mutex_ref.lock().unwrap();
                *counter += 1;
            }
        }));
    }
    // Wait for all threads to finish
    for thread in threads {
        thread.join().unwrap();
    }
    assert_eq!(*mutex.lock().unwrap(), 4_000_000);
}

/*
#[test]
fn test_mutexguard_not_send() {
    fn require_not_send<T: !Send>(_t: T) {}
    let mutex = Mutex::new(());
    require_not_send( mutex.lock().unwrap() );
}
*/
