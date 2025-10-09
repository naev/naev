use nalgebra::Vector2;

pub enum Collision<T> {
    None,
    Coincident,
    Single(Vector2<T>),
    //Double(Vector2<T>, Vector2<T>),
}

pub fn line_line(
    s1: Vector2<f64>,
    e1: Vector2<f64>,
    s2: Vector2<f64>,
    e2: Vector2<f64>,
) -> Collision<f64> {
    let ua_t = (e2.x - s2.x) * (s1.y - s2.y) - (e2.y - s2.y) * (s1.x - s2.x);
    let ub_t = (e1.x - s1.x) * (s1.y - s2.y) - (e1.y - s1.y) * (s1.x - s2.x);
    let u_b = (e2.y - s2.y) * (e1.x - s1.x) - (e2.x - s2.x) * (e1.y - s1.y);

    if u_b != 0. {
        let ua = ua_t / u_b;
        let ub = ub_t / u_b;

        // Intersection at point
        if (0. <= ua) && (ua <= 1.) && (0. <= ub) && (ub <= 1.) {
            Collision::Single(Vector2::new(
                s1.x + ua * (e1.x - s1.x),
                s1.y + ua * (e1.y - s1.y),
            ))
        }
        // No intersection, as point is off the line
        else {
            Collision::None
        }
    } else {
        // Coincident
        if (ua_t == 0.) || (ub_t == 0.) {
            Collision::Coincident
        // Parallel
        } else {
            Collision::None
        }
    }
}
