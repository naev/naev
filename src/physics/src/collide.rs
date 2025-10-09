use nalgebra::{Isometry2, Vector2};
use parry2d_f64 as parry2d;

pub enum Collision<T> {
    Coincident,
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
            Some(Collision::Coincident)
        // Parallel
        } else {
            None
        }
    }
}

// Computes collision between a line and a circle
pub fn line_circle(
    p1: Vector2<f64>,
    p2: Vector2<f64>,
    center: Vector2<f64>,
    radius: f64,
) -> Option<Collision<f64>> {
    let line = parry2d::shape::Segment::new(p1.into(), p2.into());
    let ident = Isometry2::identity();
    let circle = parry2d::shape::Ball::new(radius);
    let center = Isometry2::translation(center.x, center.y);
    match parry2d::query::contact(&ident, &line, &center, &circle, 0.0).unwrap() {
        /*
        // The point on the line gives the center collision, while the sphere gives surface
        Some(c) => Some(Collision::Double(
            c.point1.coords.into(),
            c.point2.coords.into(),
        )),
        */
        Some(c1) => {
            let line = parry2d::shape::Segment::new(p2.into(), p1.into());
            let c2 = parry2d::query::contact(&ident, &line, &center, &circle, 0.0)
                .unwrap()
                .unwrap();
            Some(Collision::Double(
                c1.point2.coords.into(),
                c2.point2.coords.into(),
            ))
        }
        None => None,
    }
}
