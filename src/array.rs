use std::os::raw::c_void;

pub fn to_vec<T: Clone>(array: *mut T) -> std::io::Result<Vec<T>> {
    unsafe {
        let len = naevc::array_size_rust(array as *const c_void);
        let out = std::slice::from_raw_parts(array, len as usize).to_vec();
        Ok(out)
    }
}
