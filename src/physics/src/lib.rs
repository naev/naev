use std::f64::consts::{PI, TAU};

pub mod transform2;
pub mod vec2;

/// Converts an angle to the [0, 2*PI] range.
pub fn angle_clean(a: f64) -> f64 {
    let mut a = a;
    if a.abs() > TAU {
        a %= TAU;
    }
    if a < 0. { a + TAU } else { a }
}

/// Gets the difference between two angles.
pub fn angle_diff(a: f64, b: f64) -> f64 {
    let d = angle_clean(b) - angle_clean(a);
    if d > PI {
        d - TAU
    } else if d < -PI {
        d + TAU
    } else {
        d
    }
}
