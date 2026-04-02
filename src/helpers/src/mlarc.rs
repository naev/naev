///! A small wrapper for both Arc<T> and Arc<RwLock<T>> so they can be used interchangeably.
///Meant to be used when loading data where one part is static while another is dynamic.
use std::ops::{Deref, DerefMut};
use std::sync::{Arc, RwLock, RwLockReadGuard, RwLockWriteGuard, Weak};

pub struct MlArc<T> {
   inner: Inner<T>,
}
enum Inner<T> {
   Shared(Arc<T>),
   Locked(Arc<RwLock<T>>),
}

impl<T> MlArc<T> {
   pub fn shared(arc: T) -> Self {
      MlArc {
         inner: Inner::Shared(Arc::new(arc)),
      }
   }

   pub fn locked(lock: T) -> Self {
      MlArc {
         inner: Inner::Locked(Arc::new(RwLock::new(lock))),
      }
   }

   pub fn borrow(&self) -> MlArcRef<'_, T> {
      let inner = match &self.inner {
         Inner::Shared(v) => MlArcRefInner::Shared(&**v),
         Inner::Locked(lock) => MlArcRefInner::Locked(lock.read().unwrap()),
      };
      MlArcRef { inner }
   }

   pub fn borrow_mut(&self) -> Option<MlArcMut<'_, T>> {
      match &self.inner {
         Inner::Shared(_) => None,
         Inner::Locked(lock) => Some(MlArcMut(lock.write().unwrap())),
      }
   }

   pub fn downgrade(&self) -> MlArcWeak<T> {
      let inner = match &self.inner {
         Inner::Shared(v) => MlArcWeakInner::Shared(Arc::downgrade(v)),
         Inner::Locked(lock) => MlArcWeakInner::Locked(Arc::downgrade(lock)),
      };
      MlArcWeak { inner }
   }

   pub fn into_raw(self) -> *mut Self {
      Box::into_raw(Box::new(self))
   }

   pub unsafe fn from_raw(ptr: *mut Self) -> Self {
      unsafe { *Box::from_raw(ptr) }
   }
}

pub struct MlArcWeak<T> {
   inner: MlArcWeakInner<T>,
}
pub enum MlArcWeakInner<T> {
   Shared(Weak<T>),
   Locked(Weak<RwLock<T>>),
}
impl<T> MlArcWeak<T> {
   pub fn upgrade(&self) -> Option<MlArc<T>> {
      match &self.inner {
         MlArcWeakInner::Shared(v) => match v.upgrade() {
            Some(v) => Some(Inner::Shared(v)),
            None => None,
         },
         MlArcWeakInner::Locked(lock) => match lock.upgrade() {
            Some(v) => Some(Inner::Locked(v)),
            None => None,
         },
      }
      .map(|inner| MlArc { inner })
   }

   pub fn into_raw(self) -> *mut Self {
      Box::into_raw(Box::new(self))
   }

   pub unsafe fn from_raw(ptr: *mut Self) -> Self {
      unsafe { *Box::from_raw(ptr) }
   }
}

pub struct MlArcRef<'a, T> {
   inner: MlArcRefInner<'a, T>,
}
enum MlArcRefInner<'a, T> {
   Shared(&'a T),
   Locked(RwLockReadGuard<'a, T>),
}
impl<'a, T> Deref for MlArcRef<'a, T> {
   type Target = T;
   fn deref(&self) -> &T {
      match &self.inner {
         MlArcRefInner::Shared(v) => v,
         MlArcRefInner::Locked(g) => &*g,
      }
   }
}
impl<'a, T> AsRef<T> for MlArcRef<'a, T> {
   fn as_ref(&self) -> &T {
      self.deref()
   }
}

pub struct MlArcMut<'a, T>(RwLockWriteGuard<'a, T>);
impl<'a, T> Deref for MlArcMut<'a, T> {
   type Target = T;
   fn deref(&self) -> &T {
      &*self.0
   }
}
impl<'a, T> DerefMut for MlArcMut<'a, T> {
   fn deref_mut(&mut self) -> &mut T {
      &mut *self.0
   }
}
impl<'a, T> AsMut<T> for MlArcMut<'a, T> {
   fn as_mut(&mut self) -> &mut T {
      self.deref_mut()
   }
}
