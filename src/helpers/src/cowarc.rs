///! A small wrapper for both Arc<T> and Arc<RwLock<T>> so they can be used interchangeably.
///Meant to be used when loading data where one part is static while another is dynamic.
use std::ops::{Deref, DerefMut};
use std::sync::{Arc, RwLock, RwLockReadGuard, RwLockWriteGuard, Weak};

pub struct CowArc<T> {
   inner: Inner<T>,
}
enum Inner<T> {
   Shared(Arc<T>),
   Locked(Arc<RwLock<T>>),
}

impl<T> CowArc<T> {
   pub fn shared(arc: T) -> Self {
      CowArc {
         inner: Inner::Shared(Arc::new(arc)),
      }
   }

   pub fn locked(lock: T) -> Self {
      CowArc {
         inner: Inner::Locked(Arc::new(RwLock::new(lock))),
      }
   }

   pub fn borrow(&self) -> CowRef<'_, T> {
      let inner = match &self.inner {
         Inner::Shared(v) => CowRefInner::Shared(&**v),
         Inner::Locked(lock) => CowRefInner::Locked(lock.read().unwrap()),
      };
      CowRef { inner }
   }

   pub fn borrow_mut(&self) -> Option<CowMut<'_, T>> {
      match &self.inner {
         Inner::Shared(_) => None,
         Inner::Locked(lock) => Some(CowMut(lock.write().unwrap())),
      }
   }

   pub fn downgrade(&self) -> CowWeak<T> {
      let inner = match &self.inner {
         Inner::Shared(v) => CowWeakInner::Shared(Arc::downgrade(v)),
         Inner::Locked(lock) => CowWeakInner::Locked(Arc::downgrade(lock)),
      };
      CowWeak { inner }
   }

   pub fn into_raw(self) -> *mut Self {
      Box::into_raw(Box::new(self))
   }

   pub unsafe fn from_raw(ptr: *mut Self) -> Self {
      unsafe { *Box::from_raw(ptr) }
   }
}

pub struct CowWeak<T> {
   inner: CowWeakInner<T>,
}
pub enum CowWeakInner<T> {
   Shared(Weak<T>),
   Locked(Weak<RwLock<T>>),
}
impl<T> CowWeak<T> {
   pub fn upgrade(&self) -> Option<CowArc<T>> {
      match &self.inner {
         CowWeakInner::Shared(v) => match v.upgrade() {
            Some(v) => Some(Inner::Shared(v)),
            None => None,
         },
         CowWeakInner::Locked(lock) => match lock.upgrade() {
            Some(v) => Some(Inner::Locked(v)),
            None => None,
         },
      }
      .map(|inner| CowArc { inner })
   }

   pub fn into_raw(self) -> *mut Self {
      Box::into_raw(Box::new(self))
   }

   pub unsafe fn from_raw(ptr: *mut Self) -> Self {
      unsafe { *Box::from_raw(ptr) }
   }
}

pub struct CowRef<'a, T> {
   inner: CowRefInner<'a, T>,
}
enum CowRefInner<'a, T> {
   Shared(&'a T),
   Locked(RwLockReadGuard<'a, T>),
}
impl<'a, T> Deref for CowRef<'a, T> {
   type Target = T;
   fn deref(&self) -> &T {
      match &self.inner {
         CowRefInner::Shared(v) => v,
         CowRefInner::Locked(g) => &*g,
      }
   }
}

pub struct CowMut<'a, T>(RwLockWriteGuard<'a, T>);
impl<'a, T> Deref for CowMut<'a, T> {
   type Target = T;
   fn deref(&self) -> &T {
      &*self.0
   }
}
impl<'a, T> DerefMut for CowMut<'a, T> {
   fn deref_mut(&mut self) -> &mut T {
      &mut *self.0
   }
}
