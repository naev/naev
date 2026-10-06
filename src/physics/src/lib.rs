use nalgebra::RealField;

pub mod solid;
pub mod transform2;
pub mod vec2;

/// Converts an angle to the [0, 2*PI] range.
pub fn angle_clean<T: RealField + Copy>(a: T) -> T {
   let mut a = a;
   if a.abs() > T::two_pi() {
      a %= T::two_pi();
   }
   if a < T::zero() { a + T::two_pi() } else { a }
}

/// Gets the difference between two angles.
pub fn angle_diff<T: RealField + Copy>(a: T, b: T) -> T {
   let d = angle_clean(b) - angle_clean(a);
   if d > T::pi() {
      d - T::two_pi()
   } else if d < -T::pi() {
      d + T::two_pi()
   } else {
      d
   }
}
