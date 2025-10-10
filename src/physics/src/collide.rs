use nalgebra::Vector2;

const TOLERANCE: f64 = 1e-6;

pub enum Collision<T> {
    Single(Vector2<T>),
    Double(Vector2<T>, Vector2<T>),
}

// Computes a collision between two line segments
pub fn line_line(
    s1: Vector2<f64>,
    e1: Vector2<f64>,
    s2: Vector2<f64>,
    e2: Vector2<f64>,
) -> Option<Collision<f64>> {
    // The parry function is more annoying to use, so we stick to our custom implementation
    // parry2d::utils::segments_intersection2d( s1.into(), e1.into(), s2.into(), e2.into() )
    let ua_t = (e2.x - s2.x) * (s1.y - s2.y) - (e2.y - s2.y) * (s1.x - s2.x);
    let ub_t = (e1.x - s1.x) * (s1.y - s2.y) - (e1.y - s1.y) * (s1.x - s2.x);
    let u_b = (e2.y - s2.y) * (e1.x - s1.x) - (e2.x - s2.x) * (e1.y - s1.y);

    if u_b != 0. {
        let ua = ua_t / u_b;
        let ub = ub_t / u_b;

        // Intersection at point
        if (0. <= ua) && (ua <= 1.) && (0. <= ub) && (ub <= 1.) {
            Some(Collision::Single(Vector2::new(
                s1.x + ua * (e1.x - s1.x),
                s1.y + ua * (e1.y - s1.y),
            )))
        }
        // No intersection, as point is off the line
        else {
            None
        }
    } else {
        // Coincident
        if (ua_t == 0.) || (ub_t == 0.) {
            //Some(Collision::Coincident)
            // Could do something smarter, but it doesn't really matter
            Some(Collision::Single((s1 + e1) * 0.5))
        // Parallel
        } else {
            None
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
) -> Option<Collision<f64>> {
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

    /* Case line in circle. */
    let r2 = cr * cr;
    if ((p1 - cc).norm_squared() < r2) && ((p2 - cc).norm_squared() < r2) {
        return Some(Collision::Single((p1 + p2) * 0.5));
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
        return None;
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
            Some(Collision::Single(Vector2::new(x, y)))
        } else {
            None
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
            let x1 = fx(A, B, C, y1);
            let y2 = (-b - d) / (2. * a);
            let x2 = fx(A, B, C, y2);
            (x1, y1, x2, y2)
        };
        let on1 = line_point_on_segment(p1, p2, d1, x1, y1);
        let on2 = line_point_on_segment(p1, p2, d1, x2, y2);
        if on1 && on2 {
            Some(Collision::Double(
                Vector2::new(x1, y1),
                Vector2::new(x2, y2),
            ))
        } else if on1 {
            Some(Collision::Single(Vector2::new(x1, y1)))
        } else if on2 {
            Some(Collision::Single(Vector2::new(x2, y2)))
        } else {
            None
        }
    }
}
