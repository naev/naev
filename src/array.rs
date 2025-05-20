use anyhow::Result;
use std::ffi::CString;
use std::os::raw::{c_char, c_void};
use std::sync::atomic::AtomicPtr;

/// Wrapper to convert C arrays to Vec
#[allow(dead_code)]
pub fn to_vec<T: Clone>(array: *mut T) -> Result<Vec<T>> {
    unsafe {
        let len = naevc::array_size_rust(array as *const c_void);
        let out = std::slice::from_raw_parts(array, len as usize).to_vec();
        Ok(out)
    }
}

/// Small wrapper for working with C arrays
pub struct Array<T: Clone>(AtomicPtr<T>);
impl<T: Clone> Default for Array<T> {
    fn default() -> Self {
        Array(AtomicPtr::default())
    }
}
impl<T: Clone + Sized> Array<T> {
    pub fn new(vec: &[T]) -> Result<Self> {
        let size = std::mem::size_of::<T>();
        let array =
            unsafe { naevc::array_from_vec(vec.as_ptr() as *const c_void, size, vec.len()) };
        if array.is_null() {
            anyhow::bail!("Failed to create C Array");
        }
        Ok(Array(AtomicPtr::new(array as *mut T)))
    }
    pub fn as_ptr(&self) -> *mut c_void {
        self.0.as_ptr() as *mut c_void
    }
}
impl<T: Clone> Drop for Array<T> {
    fn drop(&mut self) {
        unsafe {
            naevc::_array_free_helper(self.0.as_ptr() as *mut c_void);
        }
    }
}

#[derive(Default)]
pub struct ArrayCString {
    #[allow(dead_code)]
    data: Vec<CString>, // just here to store memory for the C strings
    arr: Option<Array<*const c_char>>,
}
impl ArrayCString {
    pub fn new(vec: &[String]) -> Result<Self> {
        let data: Vec<CString> = vec
            .iter()
            .map(|s| CString::new(s.as_str()).unwrap())
            .collect();
        let ptrdata: Vec<*const c_char> = data.iter().map(|s| s.as_ptr()).collect();
        let arr = Array::new(&ptrdata)?;
        Ok(ArrayCString {
            data,
            arr: Some(arr),
        })
    }
    pub fn as_ptr(&self) -> *mut *const c_char {
        match &self.arr {
            Some(arr) => arr.as_ptr() as *mut *const c_char,
            None => std::ptr::null_mut(),
        }
    }
}
impl std::fmt::Debug for ArrayCString {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> Result<(), std::fmt::Error> {
        write!(f, "ArrayCString")
    }
}
