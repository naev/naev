/// Sets the m by n matrix c to a * b, with every matrix given by its row and column strides.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn safelanes_dgemm(
   m: usize,
   k: usize,
   n: usize,
   a: *const f64,
   rsa: isize,
   csa: isize,
   b: *const f64,
   rsb: isize,
   csb: isize,
   c: *mut f64,
   rsc: isize,
   csc: isize,
) {
   // SAFETY: safelanes.c passes CHOLMOD matrices of these sizes and strides, and c is new.
   unsafe {
      matrixmultiply::dgemm(m, k, n, 1.0, a, rsa, csa, b, rsb, csb, 0.0, c, rsc, csc);
   }
}
