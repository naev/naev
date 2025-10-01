use std::f64::consts::PI;

pub mod vec2;

/// Converts an angle to the [0, 2*PI] range.
pub fn angle_clean(a: f64) -> f64 {
    let mut a = a;
    if a.abs() > 2. * PI {
        a %= 2. * PI;
    }
    if a < 0. { a + 2. * PI } else { a }
}

/// Gets the difference between two angles.
pub fn angle_diff(a: f64, b: f64) -> f64 {
    let d = angle_clean(b) - angle_clean(a);
    if d > PI {
        d - 2. * PI
    } else if d < -PI {
        d + 2. * PI
    } else {
        d
    }
}
