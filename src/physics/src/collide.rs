use arrayvec::ArrayVec;
use nalgebra::Vector2;
use nlog::warn;

pub(crate) const TOLERANCE: f64 = 1e-6;

// The parry function is more annoying to use, so we stick to our custom implementation
// parry2d::utils::segments_intersection2d( s1.into(), e1.into(), s2.into(), e2.into() )
/// Computes a collision between two line segments
pub fn line_line(
    s1: Vector2<f64>,
    e1: Vector2<f64>,
    s2: Vector2<f64>,
    e2: Vector2<f64>,
) -> ArrayVec<Vector2<f64>, 2> {
    // Handle degenerate line cases
    if s1 == e1 || s2 == e2 {
        return ArrayVec::new();
    }

    let ua_t = (e2.x - s2.x) * (s1.y - s2.y) - (e2.y - s2.y) * (s1.x - s2.x);
    let ub_t = (e1.x - s1.x) * (s1.y - s2.y) - (e1.y - s1.y) * (s1.x - s2.x);
    let u_b = (e2.y - s2.y) * (e1.x - s1.x) - (e2.x - s2.x) * (e1.y - s1.y);

    if u_b.abs() > TOLERANCE {
        let ua = ua_t / u_b;
        let ub = ub_t / u_b;

        // Intersection at point
        if (0. ..=1.).contains(&ua) && (0. ..=1.).contains(&ub) {
            let mut out = ArrayVec::new();
            out.push(Vector2::new(
                s1.x + ua * (e1.x - s1.x),
                s1.y + ua * (e1.y - s1.y),
            ));
            out
        }
        // No intersection, as point is off the line
        else {
            ArrayVec::new()
        }
    } else {
        // Coincident
        if (ua_t.abs() <= TOLERANCE) && (ub_t.abs() <= TOLERANCE) {
            let d = e1 - s1;

            // Choose projection axis to avoid division by near-zero
            let use_x = d.x.abs() >= d.y.abs();

            let (a0, a1, b0, b1) = if use_x {
                (s1.x, e1.x, s2.x, e2.x)
            } else {
                (s1.y, e1.y, s2.y, e2.y)
            };

            let (a_min, a_max) = if a0 <= a1 { (a0, a1) } else { (a1, a0) };
            let (b_min, b_max) = if b0 <= b1 { (b0, b1) } else { (b1, b0) };

            let lo = a_min.max(b_min);
            let hi = a_max.min(b_max);

            let mut out = ArrayVec::new();

            // No overlap
            if lo > hi + TOLERANCE {
                return out;
            }

            // Map back to points
            let t_lo = if use_x {
                (lo - s1.x) / d.x
            } else {
                (lo - s1.y) / d.y
            };

            let p_lo = s1 + d * t_lo;
            out.push(p_lo);

            if (hi - lo).abs() > TOLERANCE {
                let t_hi = if use_x {
                    (hi - s1.x) / d.x
                } else {
                    (hi - s1.y) / d.y
                };
                let p_hi = s1 + d * t_hi;
                out.push(p_hi);
            }
            out
        // Parallel
        } else {
            ArrayVec::new()
        }
    }
}

#[allow(non_snake_case)]
/// Computes collision between a line and a circle
pub fn line_circle(
    p1: Vector2<f64>,
    p2: Vector2<f64>,
    cc: Vector2<f64>,
    cr: f64,
) -> ArrayVec<Vector2<f64>, 2> {
    fn fx(A: f64, B: f64, C: f64, x: f64) -> f64 {
        -(A * x + C) / B
    }
    fn fy(A: f64, B: f64, C: f64, y: f64) -> f64 {
        -(B * y + C) / A
    }
    fn line_point_on_segment(p1: Vector2<f64>, p2: Vector2<f64>, d1: f64, x: f64, y: f64) -> bool {
        let v = Vector2::new(x, y);
        let d2 = (v - p1).norm();
        let d3 = (v - p2).norm();
        (d1 - d2 - d3).abs() < TOLERANCE
    }

    /* Case line is completely in circle. */
    let r2 = cr * cr;
    if ((p1 - cc).norm_squared() < r2) && ((p2 - cc).norm_squared() < r2) {
        return ArrayVec::from([p1, p2]);
    }

    let A = p2.y - p1.y;
    let B = p1.x - p2.x;
    let C = p2.x * p1.y - p1.x * p2.y;

    let a = A * A + B * B;

    // Non-vertical case.
    let (b, c, bnz) = if B.abs() >= TOLERANCE {
        let b = 2. * (A * C + A * B * cc.y - B * B * cc.x);
        let c = C * C + 2. * B * C * cc.y - B * B * (cr * cr - cc.norm_squared());
        (b, c, true)
    // Have to have special care when line is vertical.
    } else {
        let b = 2. * (B * C + A * B * cc.x - A * A * cc.y);
        let c = C * C + 2. * A * C * cc.x - A * A * (cr * cr - cc.norm_squared());
        (b, c, false)
    };

    // Discriminant
    let d = b * b - 4. * a * c;
    if d < 0. {
        return ArrayVec::new();
    }

    let d1 = (p2 - p1).norm();
    if d.abs() < TOLERANCE {
        let (x, y) = if bnz {
            let x = -b / (2. * a);
            let y = fx(A, B, C, x);
            (x, y)
        } else {
            let y = -b / (2. * a);
            let x = fy(A, B, C, y);
            (x, y)
        };
        if line_point_on_segment(p1, p2, d1, x, y) {
            let mut out = ArrayVec::new();
            out.push(Vector2::new(x, y));
            out
        } else {
            ArrayVec::new()
        }

    // Two potential intersections
    } else {
        let d = d.sqrt();
        let (x1, y1, x2, y2) = if bnz {
            let x1 = (-b + d) / (2. * a);
            let y1 = fx(A, B, C, x1);
            let x2 = (-b - d) / (2. * a);
            let y2 = fx(A, B, C, x2);
            (x1, y1, x2, y2)
        } else {
            let y1 = (-b + d) / (2. * a);
            let x1 = fy(A, B, C, y1);
            let y2 = (-b - d) / (2. * a);
            let x2 = fy(A, B, C, y2);
            (x1, y1, x2, y2)
        };
        let on1 = line_point_on_segment(p1, p2, d1, x1, y1);
        let on2 = line_point_on_segment(p1, p2, d1, x2, y2);
        if on1 && on2 {
            ArrayVec::from([Vector2::new(x1, y1), Vector2::new(x2, y2)])
        } else if on1 {
            let mut out = ArrayVec::new();
            out.push(Vector2::new(x1, y1));
            out
        } else if on2 {
            let mut out = ArrayVec::new();
            out.push(Vector2::new(x2, y2));
            out
        } else {
            ArrayVec::new()
        }
    }
}

pub fn circle_circle(
    p1: Vector2<f64>,
    r1: f64,
    p2: Vector2<f64>,
    r2: f64,
) -> ArrayVec<Vector2<f64>, 2> {
    let mut hit = ArrayVec::new();
    let d2 = (p2 - p1).norm_squared();
    if d2 > r1 * r1 + r2 * r2 {
        return hit;
    }
    hit.push((p1 * r1 + p2 * r2) / (r1 + r2));
    hit
}

// C API
use std::ffi::{c_double, c_int};

#[unsafe(no_mangle)]
pub extern "C" fn collide_line_line(
    s1x: c_double,
    s1y: c_double,
    e1x: c_double,
    e1y: c_double,
    s2x: c_double,
    s2y: c_double,
    e2x: c_double,
    e2y: c_double,
    crash: *mut Vector2<f64>,
) -> c_int {
    let hit = line_line(
        Vector2::new(s1x, s1y),
        Vector2::new(e1x, e1y),
        Vector2::new(s2x, s2y),
        Vector2::new(e2x, e2y),
    );
    if !crash.is_null() {
        let crash: &mut [Vector2<f64>] = unsafe { std::slice::from_raw_parts_mut(crash, 2) };
        for (k, h) in hit.iter().enumerate() {
            crash[k] = *h;
        }
    }
    hit.len() as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn collide_line_circle(
    p1: *const Vector2<f64>,
    p2: *const Vector2<f64>,
    cc: *const Vector2<f64>,
    cr: c_double,
    crash: *mut Vector2<f64>,
) -> c_int {
    if p1.is_null() || p2.is_null() || cc.is_null() {
        warn!("got null pointer");
        return 0;
    }
    let p1 = unsafe { &*p1 };
    let p2 = unsafe { &*p2 };
    let cc = unsafe { &*cc };
    let hit = line_circle(*p1, *p2, *cc, cr);
    if !crash.is_null() {
        let crash: &mut [Vector2<f64>] = unsafe { std::slice::from_raw_parts_mut(crash, 2) };
        for (k, h) in hit.iter().enumerate() {
            crash[k] = *h;
        }
    }
    hit.len() as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn collide_circle_circle(
    p1: *const Vector2<f64>,
    r1: c_double,
    p2: *const Vector2<f64>,
    r2: c_double,
    crash: *mut Vector2<f64>,
) -> c_int {
    if p1.is_null() || p2.is_null() {
        warn!("got null pointer");
        return 0;
    }
    let p1 = unsafe { &*p1 };
    let p2 = unsafe { &*p2 };
    let hit = circle_circle(*p1, r1, *p2, r2);
    if !crash.is_null() {
        let crash: &mut [Vector2<f64>] = unsafe { std::slice::from_raw_parts_mut(crash, 2) };
        for (k, h) in hit.iter().enumerate() {
            crash[k] = *h;
        }
    }
    hit.len() as c_int
}

#[cfg(test)]
mod tests {
    use super::*;
    use nalgebra::Vector2;

    fn v(x: f64, y: f64) -> Vector2<f64> {
        Vector2::new(x, y)
    }

    fn v_eq(a: Vector2<f64>, b: Vector2<f64>) -> bool {
        (a - b).norm() < TOLERANCE
    }

    #[test]
    fn line_line_cross() {
        let p = line_line(v(0.0, 0.0), v(2.0, 2.0), v(0.0, 2.0), v(2.0, 0.0));
        assert_eq!(p.len(), 1);
        assert_eq!(p[0], v(1.0, 1.0));
    }

    #[test]
    fn line_line_t() {
        let p = line_line(v(0.0, 2.0), v(2.0, 2.0), v(1.0, 2.0), v(1.0, 0.0));
        assert_eq!(p.len(), 1);
        assert_eq!(p[0], v(1.0, 2.0));
    }

    #[test]
    fn line_line_t_disjoint() {
        let p = line_line(v(0.0, 2.0), v(2.0, 2.0), v(1.0, 1.9), v(1.0, 0.0));
        assert_eq!(p.len(), 0);
    }

    #[test]
    fn line_line_parallel() {
        let p = line_line(v(0.0, 0.0), v(2.0, 0.0), v(0.0, 1.0), v(2.0, 1.0));
        assert!(p.is_empty());
    }

    #[test]
    fn line_line_point() {
        let p = line_line(v(0.0, 0.0), v(2.0, 0.0), v(2.0, 0.0), v(2.0, 2.0));
        assert_eq!(p.len(), 1);
        assert_eq!(p[0], v(2.0, 0.0));
    }

    #[test]
    fn line_line_colinear_disjoint() {
        let p = line_line(v(0.0, 0.0), v(1.0, 0.0), v(2.0, 0.0), v(3.0, 0.0));
        assert!(p.is_empty());
    }

    #[test]
    fn line_line_colinear_overlap() {
        let p = line_line(v(0.0, 0.0), v(3.0, 0.0), v(1.0, 0.0), v(2.0, 0.0));
        assert_eq!(p.len(), 2);
        assert_eq!(p[0], v(1.0, 0.0));
        assert_eq!(p[1], v(2.0, 0.0));
    }

    #[test]
    fn line_circle_two() {
        let p = line_circle(v(-2.0, 0.0), v(2.0, 0.0), v(0.0, 0.0), 1.0);
        assert_eq!(p.len(), 2);
        assert!(
            (v_eq(p[0], v(-1.0, 0.0)) && v_eq(p[1], v(1.0, 0.0)))
                || (v_eq(p[0], v(1.0, 0.0)) && v_eq(p[1], v(-1.0, 0.0)))
        );
    }

    #[test]
    fn line_circle_one() {
        let p = line_circle(v(0.0, 0.0), v(2.0, 0.0), v(0.0, 0.0), 1.0);
        assert_eq!(p.len(), 1);
        assert_eq!(p[0], v(1.0, 0.0));
    }

    // Not intersecting, but we want to consider it as a hit for collision purposes
    #[test]
    fn line_circle_inside() {
        let p = line_circle(v(-0.5, 0.0), v(0.5, 0.0), v(0.0, 0.0), 1.0);
        assert_eq!(p.len(), 2);
        assert_eq!(p[0], v(-0.5, 0.0));
        assert_eq!(p[1], v(0.5, 0.0));
    }

    #[test]
    fn line_circle_tangent() {
        let p = line_circle(v(-2.0, 1.0), v(2.0, 1.0), v(0.0, 0.0), 1.0);
        assert_eq!(p.len(), 1);
        assert_eq!(p[0], v(0.0, 1.0));
    }

    #[test]
    fn line_circle_miss() {
        let p = line_circle(v(2.0, 0.0), v(3.0, 0.0), v(0.0, 0.0), 1.0);
        assert!(p.is_empty());
    }
}
