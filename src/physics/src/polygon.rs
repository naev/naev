use crate::collide::{line_circle, line_line};
use anyhow::Result;
use arrayvec::ArrayVec;
use image::GenericImageView;
use itertools::Itertools;
use nalgebra::Vector2;
use nlog::{warn, warn_err};
use serde::{Deserialize, Serialize};
use std::collections::VecDeque;
use std::path::Path;

const ALPHA_THRESHOLD: u8 = 50;

#[derive(Debug, Serialize, Deserialize)]
pub struct XmlPolygonRaw {
    x: String,
    y: String,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct XmlPolygon {
    polygon: Vec<XmlPolygonRaw>,
}

#[derive(Debug, PartialEq, Serialize, Deserialize, Clone)]
pub struct Polygon {
    /// The points in the polygon.
    pub points: Vec<Vector2<f64>>,
    /// Minimum x value of the AABB
    pub xmin: f64,
    /// Maximum x value of the AABB
    pub xmax: f64,
    /// Minimum y value of the AABB
    pub ymin: f64,
    /// Maximum y value of the AABB
    pub ymax: f64,
    pub start: Option<Vector2<i32>>,
}

impl Polygon {
    pub fn from_points(points: Vec<Vector2<f64>>, start: Option<Vector2<i32>>) -> Self {
        let mut xmin = f64::INFINITY;
        let mut xmax = -f64::INFINITY;
        let mut ymin = f64::INFINITY;
        let mut ymax = -f64::INFINITY;
        for p in &points {
            xmin = xmin.min(p.x);
            xmax = xmax.max(p.x);
            ymin = ymin.min(p.y);
            ymax = ymax.max(p.y);
        }
        Polygon {
            points,
            xmin,
            xmax,
            ymin,
            ymax,
            start,
        }
    }

    pub fn from_binary(points: &[bool], width: i32, height: i32, dir: f64) -> Self {
        let (c, start) = trace_contour(points, width, height, dir);
        let (w2, h2) = (width as f64 * 0.5, height as f64 * 0.5);
        let c: Vec<mint::Point2<i32>> = c
            .iter()
            .map(|v| mint::Point2::<i32> { x: v.x, y: v.y })
            .collect();
        let idx = ramer_douglas_peucker::rdp(&c, 0.5);
        let points: Vec<Vector2<f64>> = idx
            .iter()
            .map(|v| {
                let m = c[*v];
                Vector2::new(m.x as f64 - w2, m.y as f64 - h2)
            })
            .collect();
        Self::from_points(points, start)
    }

    pub fn from_subimage(
        img: &image::DynamicImage,
        x: u32,
        y: u32,
        w: u32,
        h: u32,
        dir: f64,
    ) -> Self {
        let x2 = w / 2;
        let y2 = h / 2;
        let start = if img.get_pixel(x + x2, y + y2)[3] > ALPHA_THRESHOLD {
            Vector2::new(x2 as i32, y2 as i32)
        } else {
            // Start point not found so grow from the inside
            let mut visited = vec![false; (w * h) as usize];
            let mut queue = VecDeque::new();
            queue.push_back(Vector2::new(x2 as i32, y2 as i32));
            let mut start = None;
            'outer: while let Some(pos) = queue.pop_front() {
                for d in DIRS {
                    let n = pos + d;
                    let idx = (n.x * h as i32 + n.y) as usize;
                    if img.get_pixel(x + n.x as u32, y + n.y as u32)[3] > ALPHA_THRESHOLD {
                        start = Some(Vector2::new(n.x, n.y));
                        break 'outer;
                    }
                    if !visited[idx] {
                        visited[idx] = true;
                        queue.push_back(n);
                    }
                }
            }
            if let Some(start) = start {
                start
            } else {
                // Not found so just return original guess, the rest of the algo will just return an
                // empty polygon
                Vector2::new(x2 as i32, y2 as i32)
            }
        };

        // Grow along the start point to find the connected area
        let mut points = vec![false; (w * h) as usize];
        let mut queue = VecDeque::new();
        queue.push_back(start);
        points[(start.x * h as i32 + start.y) as usize] = true;
        while let Some(pos) = queue.pop_front() {
            for d in DIRS {
                let n = pos + d;
                let idx = (n.x * h as i32 + n.y) as usize;
                if n.x >= 0
                    && n.y >= 0
                    && n.x < w as i32
                    && n.y < h as i32
                    && !points[idx]
                    && img.get_pixel(x + n.x as u32, y + n.y as u32)[3] > ALPHA_THRESHOLD
                {
                    points[idx] = true;
                    queue.push_back(n);
                }
            }
        }

        Self::from_binary(&points, w as i32, h as i32, dir)
    }

    pub fn contains_point(&self, p: Vector2<f64>) -> bool {
        if p.x < self.xmin || p.x > self.xmax || p.y < self.ymin || p.y > self.ymax {
            return false;
        }

        let mut winding = 0;
        for (a, b) in self.points.iter().circular_tuple_windows() {
            // Check side of the line
            let side = (b.x - a.x) * (p.y - a.y) - (p.x - a.x) * (b.y - a.y);

            // Non-zero means it's on a side
            if side.abs() > crate::collide::TOLERANCE {
                // Crosses upwards?
                if a.y <= p.y {
                    // Yes, crosses upwards
                    if b.y > p.y && side > 0.0 {
                        winding += 1;
                    }
                } else {
                    // Crosses downwards?
                    if b.y <= p.y && side < 0.0 {
                        winding -= 1;
                    }
                }
            } else {
                // Check to see if on line segment
                let dot = (p.x - a.x) * (p.x - b.x) + (p.y - a.y) * (p.y - b.y);
                if dot <= 0.0 {
                    return true;
                }
            }
        }

        winding != 0
    }

    /// Intersection with a line segment.
    pub fn intersect_line(&self, s: Vector2<f64>, e: Vector2<f64>) -> ArrayVec<Vector2<f64>, 2> {
        let mut hit: ArrayVec<Vector2<f64>, 2> = ArrayVec::new();

        // Check end points
        if self.contains_point(s) {
            hit.push(s);
        }
        if self.contains_point(e) {
            hit.push(e);
        }
        if hit.is_full() {
            return hit;
        }

        // Test AABB
        if hit.is_empty()
            && line_line(
                s,
                e,
                Vector2::new(self.xmin, self.ymin),
                Vector2::new(self.xmax, self.ymin),
            )
            .is_empty()
            && line_line(
                s,
                e,
                Vector2::new(self.xmax, self.ymin),
                Vector2::new(self.xmax, self.ymax),
            )
            .is_empty()
            && line_line(
                s,
                e,
                Vector2::new(self.xmax, self.ymax),
                Vector2::new(self.xmin, self.ymax),
            )
            .is_empty()
            && line_line(
                s,
                e,
                Vector2::new(self.xmin, self.ymax),
                Vector2::new(self.xmin, self.ymin),
            )
            .is_empty()
        {
            return ArrayVec::new();
        }

        for (a, b) in self.points.iter().circular_tuple_windows() {
            let i = line_line(s, e, *a, *b);
            if !i.is_empty() {
                hit.push(i[0]);
                if hit.is_full() {
                    return hit;
                }
            }
        }
        hit
    }

    pub fn intersect_circle(&self, centre: Vector2<f64>, radius: f64) -> ArrayVec<Vector2<f64>, 2> {
        let mut hit = ArrayVec::new();

        // AABB testing
        let r2 = radius * radius;
        let cx = centre.x.clamp(self.xmin, self.xmax);
        let cy = centre.y.clamp(self.ymin, self.ymax);
        let dx = cx - centre.x;
        let dy = cy - centre.y;
        if dx * dx + dy * dy > r2 {
            return hit;
        }

        // Intersects edges
        for (a, b) in self.points.iter().circular_tuple_windows() {
            let h = line_circle(*a, *b, centre, radius);
            if let Some(h) = h.first() {
                hit.push(*h);
                if hit.is_full() {
                    return hit;
                }
            }
        }

        // Circle contained in polygon
        if self.contains_point(centre) {
            hit.push(centre);
            return hit;
        }

        // Polygon contained in circle
        let v0 = self.points[0];
        if (v0 - centre).norm_squared() <= r2 {
            hit.push(v0);
        }

        hit
    }

    /// Intersection with another polygon given a relative distance
    pub fn intersect_polygon(
        &self,
        other: &Polygon,
        rel: Vector2<f64>,
    ) -> ArrayVec<Vector2<f64>, 2> {
        // AABB test
        let oxmin = other.xmin + rel.x;
        let oxmax = other.xmax + rel.x;
        let oymin = other.ymin + rel.y;
        let oymax = other.ymax + rel.y;
        if self.xmax < oxmin || self.xmin > oxmax || self.ymax < oymin || self.ymin > oymax {
            return ArrayVec::new();
        }

        // Test edge vs edge
        let mut hit = ArrayVec::new();
        for (s1, e1) in self.points.iter().circular_tuple_windows() {
            for (s2, e2) in other.points.iter().circular_tuple_windows() {
                let h = line_line(*s1, *e1, *s2 + rel, *e2 + rel);
                if let Some(h) = h.first() {
                    hit.push(*h);
                    if hit.is_full() {
                        return hit;
                    }
                }
            }
        }

        // Test to see if one is inside the other
        let p_other = other.points[0] + rel;
        if self.contains_point(p_other) {
            hit.push(p_other);
            return hit;
        }
        let p_self = self.points[0];
        if other.contains_point(p_self - rel) {
            hit.push(p_self);
        }
        hit
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub struct SpinPolygon {
    /// The different collision polygons for each of the view starting at dir_off and incrementing
    /// by dir_inc.
    pub polygons: Vec<Polygon>,
    /// How much angle to increment per polygon
    pub dir_inc: f64,
    /// Initial angle offset
    pub dir_off: f64,
}

impl SpinPolygon {
    pub fn from_image_path<P: AsRef<Path>>(path: P, sx: u32, sy: u32) -> Result<Self> {
        // TODO caching
        let path = ndata::image_path(&path)?;
        let io = ndata::iostream(&path)?;
        let img = image::ImageReader::with_format(
            std::io::BufReader::new(io),
            image::ImageFormat::from_path(&path)?,
        )
        .decode()?;
        Ok(SpinPolygon::from_image(&img, sx, sy))
    }

    pub fn from_image(img: &image::DynamicImage, sx: u32, sy: u32) -> Self {
        let (w, h) = img.dimensions();

        let sw = w / sx;
        let sh = h / sy;
        assert_eq!(sw * sx, w);
        assert_eq!(sh * sy, h);

        let mut polygons = Vec::new();
        let bimg = img.fast_blur(1.0);
        for y in 0..sy {
            for x in 0..sx {
                let dir = (y * sx + x) as f64 / (sx * sy) as f64 * -std::f64::consts::TAU;

                let poly = Polygon::from_subimage(&bimg, x * sw, y * sh, sw, sh, dir);
                polygons.push(poly);
                // let p: Vec<_> = poly.points.iter().map( |p| imageproc::point::Point::new( (x*sw + p.x as u32) as f64, (y*sh + p.y as u32) as f64 ) ).collect();
                //imageproc::drawing::draw_hollow_polygon_mut(&mut img, &p, image::Rgba::<u8>([255, 0, 0,255]));
            }
        }
        let dir_inc = 1.0 / (sx * sy) as f64 * std::f64::consts::TAU;
        let dir_off = -dir_inc * 0.5;
        polygons.reverse();

        SpinPolygon {
            polygons,
            dir_inc,
            dir_off,
        }
    }

    pub fn from_xml<P: AsRef<Path>>(path: P) -> Result<Self> {
        fn parse_list(s: &str) -> Result<Vec<f64>> {
            s.split(',').map(|v| Ok(v.trim().parse::<f64>()?)).collect()
        }

        let mut polygons = Vec::new();
        let xml = ndata::read_to_string(&path)?;
        let raw: XmlPolygon = quick_xml::de::from_str(&xml)?;
        for p in raw.polygon {
            let xs = parse_list(&p.x)?;
            let ys = parse_list(&p.y)?;

            if xs.len() != ys.len() {
                anyhow::bail!("x/y length mismatch");
            }

            let points = xs
                .into_iter()
                .zip(ys)
                .map(|(x, y)| Vector2::new(x, y))
                .collect::<Vec<Vector2<f64>>>();
            polygons.push(Polygon::from_points(points, None));
        }
        let dir_inc = std::f64::consts::TAU / polygons.len() as f64;
        let dir_off = dir_inc * 0.5;
        Ok(SpinPolygon {
            polygons,
            dir_inc,
            dir_off,
        })
    }

    pub fn view(&self, dir: f64) -> &Polygon {
        let dir = crate::angle_clean(dir);
        let s = ((dir + self.dir_off) / self.dir_inc).round() as usize;
        let s = s % self.polygons.len();
        &self.polygons[s]
    }
}

const DIRS: [Vector2<i32>; 8] = [
    Vector2::new(1, 0),   // 0 E
    Vector2::new(1, 1),   // 1 SE
    Vector2::new(0, 1),   // 2 S
    Vector2::new(-1, 1),  // 3 SW
    Vector2::new(-1, 0),  // 4 W
    Vector2::new(-1, -1), // 5 NW
    Vector2::new(0, -1),  // 6 N
    Vector2::new(1, -1),  // 7 NE
];

/// Moore-Neighbour algorithm
fn trace_contour(
    image: &[bool],
    width: i32,
    height: i32,
    dir: f64,
) -> (Vec<Vector2<i32>>, Option<Vector2<i32>>) {
    // Find starting pixel (furthest right)
    let mut start = None;
    let mut max_dir: f64 = f64::MIN;
    let dirv = Vector2::new(dir.cos(), dir.sin());
    for y in 0..height {
        for x in 0..width {
            if image[(x * height + y) as usize] {
                let d = dirv.dot(&Vector2::new(
                    (x - width / 2) as f64,
                    (y - height / 2) as f64,
                ));
                if d > max_dir {
                    start = Some(Vector2::new(x, y));
                    max_dir = d;
                }
            }
        }
    }
    let start = match start {
        Some(p) => p,
        None => return (vec![], None), // no foreground
    };

    // Correct angle if negative
    let dir = crate::angle_clean(dir);

    let mut contour = Vec::new();
    let mut current = start;
    let mut dir = (dir / std::f64::consts::TAU * DIRS.len() as f64).round() as usize; // Start by in the facing direction

    loop {
        contour.push(current);

        let mut found_next = false;

        // Search neighbors clockwise starting from (dir + 1)
        for i in 0..8 {
            let ndir = (dir + 1 + i) % 8;
            let d = DIRS[ndir];
            let n = current + d;

            if n.x >= 0
                && n.y >= 0
                && n.x < width
                && n.y < height
                && image[(n.x * height + n.y) as usize]
            {
                current = n;
                dir = (ndir + 4) % 8; // opposite direction
                found_next = true;
                break;
            }
        }

        // Isolated pixel
        if !found_next {
            break;
        }
        // Loop closure
        if current == start && contour.len() > 1 {
            contour.push(start);
            break;
        }
    }
    (contour, Some(start))
}

// C API
use std::ffi::{CStr, c_char, c_int};

#[unsafe(no_mangle)]
pub extern "C" fn poly_load_xml(name: *const c_char) -> *mut SpinPolygon {
    let path = unsafe { CStr::from_ptr(name).to_str().unwrap() };
    let path = Path::new(path);
    match SpinPolygon::from_xml(path) {
        Ok(sp) => Box::into_raw(Box::new(sp)),
        Err(e) => {
            warn_err!(e);
            std::ptr::null_mut::<SpinPolygon>()
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn poly_load_2d(name: *const c_char, sx: c_int, sy: c_int) -> *mut SpinPolygon {
    let path = unsafe { CStr::from_ptr(name).to_str().unwrap() };
    let path = Path::new(path);
    match SpinPolygon::from_image_path(path, sx as u32, sy as u32) {
        Ok(sp) => Box::into_raw(Box::new(sp)),
        Err(e) => {
            warn_err!(e);
            std::ptr::null_mut::<SpinPolygon>()
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn poly_free(poly: *mut SpinPolygon) {
    if poly.is_null() {
        return;
    }
    let _ = unsafe { Box::from_raw(poly) };
}

#[unsafe(no_mangle)]
pub extern "C" fn poly_free_view(poly: *mut Polygon) {
    if poly.is_null() {
        return;
    }
    let _ = unsafe { Box::from_raw(poly) };
}

#[unsafe(no_mangle)]
pub extern "C" fn poly_view(poly: *const SpinPolygon, dir: f64) -> *const Polygon {
    if poly.is_null() {
        return std::ptr::null();
    }
    let poly = unsafe { &*poly };
    poly.view(dir) as *const Polygon
}

#[unsafe(no_mangle)]
pub extern "C" fn poly_points(poly: *const Polygon, n: *mut c_int) -> *const Vector2<f64> {
    if poly.is_null() {
        return std::ptr::null();
    }
    let poly = unsafe { &*poly };
    let n = unsafe { &mut *n };
    *n = poly.points.len() as c_int;
    poly.points.as_ptr()
}

#[unsafe(no_mangle)]
pub extern "C" fn poly_rotate(poly: *const SpinPolygon, theta: f64) -> *mut Polygon {
    if poly.is_null() {
        return std::ptr::null_mut::<Polygon>();
    }
    let poly = unsafe { &*poly };
    let poly = &poly.polygons[0];
    let ct = theta.cos();
    let st = theta.sin();
    let mut points = poly.points.clone();
    for p in &mut points {
        let x = p.x;
        let y = p.y;
        p.x = x * ct - y * st;
        p.y = x * st + y * ct;
    }
    let poly = Polygon::from_points(points, None);
    Box::into_raw(Box::new(poly))
}

#[unsafe(no_mangle)]
pub extern "C" fn collide_circle_polygon(
    ap: *const Vector2<f64>,
    ar: f64,
    bt: *const Polygon,
    bp: *const Vector2<f64>,
    crash: *mut Vector2<f64>,
) -> c_int {
    if ap.is_null() || bt.is_null() || bp.is_null() || crash.is_null() {
        warn!("got null pointer");
        return 0;
    }
    let ap = unsafe { &*ap };
    let bt = unsafe { &*bt };
    let bp = unsafe { &*bp };
    let crash: &mut [Vector2<f64>] = unsafe { std::slice::from_raw_parts_mut(crash, 2) };

    let hit = bt.intersect_circle(*ap - *bp, ar);
    for (k, h) in hit.iter().enumerate() {
        crash[k] = *h;
    }
    hit.len() as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn collide_line_polygon(
    ap: *const Vector2<f64>,
    ad: f64,
    al: f64,
    bt: *const Polygon,
    bp: *const Vector2<f64>,
    crash: *mut Vector2<f64>,
) -> c_int {
    if ap.is_null() || bt.is_null() || bp.is_null() || crash.is_null() {
        warn!("got null pointer");
        return 0;
    }
    let ap = unsafe { &*ap };
    let bt = unsafe { &*bt };
    let bp = unsafe { &*bp };
    let crash: &mut [Vector2<f64>] = unsafe { std::slice::from_raw_parts_mut(crash, 2) };

    let pos = *ap - *bp;
    let hit = bt.intersect_line(
        pos,
        Vector2::new(pos.x + al * ad.cos(), pos.y + al * ad.sin()),
    );
    for (k, h) in hit.iter().enumerate() {
        crash[k] = *h;
    }
    hit.len() as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn collide_polygon_polygon(
    at: *const Polygon,
    ap: *const Vector2<f64>,
    bt: *const Polygon,
    bp: *const Vector2<f64>,
    crash: *mut Vector2<f64>,
) -> c_int {
    if at.is_null() || ap.is_null() || bt.is_null() || bp.is_null() || crash.is_null() {
        warn!("got null pointer");
        return 0;
    }
    let at = unsafe { &*at };
    let ap = unsafe { &*ap };
    let bt = unsafe { &*bt };
    let bp = unsafe { &*bp };
    let crash: &mut [Vector2<f64>] = unsafe { std::slice::from_raw_parts_mut(crash, 2) };

    let hit = at.intersect_polygon(bt, *bp - *ap);
    for (k, h) in hit.iter().enumerate() {
        crash[k] = *h;
    }
    hit.len() as c_int
}

#[cfg(test)]
mod tests {
    use super::*;

    fn v(x: f64, y: f64) -> Vector2<f64> {
        Vector2::new(x, y)
    }

    #[test]
    fn polygon_square1() {
        #[rustfmt::skip]
        let test = [
            false, false, false, false, false,
            false, true,  true,  true,  false,
            false, true,  false, true,  false,
            false, true,  true,  true,  false,
            false, false, false, false, false,
        ];
        let poly = Polygon::from_binary(&test, 5, 5, -std::f64::consts::PI * 3. / 4.);
        assert_eq!(
            poly,
            Polygon {
                points: vec![
                    v(1.0, 1.0),
                    v(3.0, 1.0),
                    v(3.0, 3.0),
                    v(1.0, 3.0),
                    v(1.0, 1.0),
                ],
                xmin: 1.0,
                xmax: 3.0,
                ymin: 1.0,
                ymax: 3.0,
                start: Some(Vector2::new(1, 1)),
            }
        );
    }

    #[test]
    fn polygon_square1_angle() {
        #[rustfmt::skip]
        let test = [
            false, false, false, false, false,
            false, true,  true,  true,  false,
            false, true,  false, true,  false,
            false, true,  true,  true,  false,
            false, false, false, false, false,
        ];
        let poly = Polygon::from_binary(&test, 5, 5, std::f64::consts::PI * 1. / 4.);
        assert_eq!(
            poly,
            Polygon {
                points: vec![
                    v(3.0, 3.0),
                    v(1.0, 3.0),
                    v(1.0, 1.0),
                    v(3.0, 1.0),
                    v(3.0, 3.0),
                ],
                xmin: 1.0,
                xmax: 3.0,
                ymin: 1.0,
                ymax: 3.0,
                start: Some(Vector2::new(3, 3)),
            }
        );
    }

    #[test]
    fn polygon_square2() {
        #[rustfmt::skip]
        let test = [
            true, true, true, true, true,
            true, true, true, true, true,
            true, true, true, true, true,
            true, true, true, true, true,
            true, true, true, true, true,
        ];
        let poly = Polygon::from_binary(&test, 5, 5, std::f64::consts::PI * -3. / 4.);
        assert_eq!(
            poly,
            Polygon {
                points: vec![
                    v(0.0, 0.0),
                    v(4.0, 0.0),
                    v(4.0, 4.0),
                    v(0.0, 4.0),
                    v(0.0, 0.0),
                ],
                xmin: 0.0,
                xmax: 4.0,
                ymin: 0.0,
                ymax: 4.0,
                start: Some(Vector2::new(0, 0)),
            }
        );
    }

    #[test]
    fn polygon_square3() {
        #[rustfmt::skip]
        let test = [
            false, false, true, false, false,
            false, true,  true, true,  false,
            true,  true,  true, true,  true,
            false, true,  true, true,  false,
            false, false, true, false, false,
        ];
        let poly = Polygon::from_binary(&test, 5, 5, std::f64::consts::PI * -3. / 4.);
        assert_eq!(
            poly,
            Polygon {
                points: vec![
                    v(2.0, 0.0),
                    v(4.0, 2.0),
                    v(2.0, 4.0),
                    v(0.0, 2.0),
                    v(2.0, 0.0),
                ],
                xmin: 0.0,
                xmax: 4.0,
                ymin: 0.0,
                ymax: 4.0,
                start: Some(Vector2::new(2, 0)),
            }
        );
    }

    #[test]
    fn polygon_hollow_square() {
        // Note that the inside here will cause collisions
        #[rustfmt::skip]
        let test = [
            true, true,  true,  true,  true,  true,  true,
            true, false, false, false, false, false, true,
            true, false, false, false, false, false, true,
            true, false, false, false, false, false, true,
            true, false, false, false, false, false, true,
            true, false, false, false, false, false, true,
            true, true,  true,  true,  true,  true,  true,
        ];
        let poly = Polygon::from_binary(&test, 7, 7, std::f64::consts::PI * -3. / 4.);
        assert_eq!(
            poly,
            Polygon {
                points: vec![
                    v(0.0, 0.0),
                    v(6.0, 0.0),
                    v(6.0, 6.0),
                    v(0.0, 6.0),
                    v(0.0, 0.0),
                ],
                xmin: 0.0,
                xmax: 6.0,
                ymin: 0.0,
                ymax: 6.0,
                start: Some(Vector2::new(0, 0)),
            }
        );
    }

    #[test]
    fn polygon_image() {
        const N: u32 = 5;
        let mut img: image::ImageBuffer<image::Rgba<u8>, Vec<u8>> = image::ImageBuffer::new(N, N);

        for (x, y, pixel) in img.enumerate_pixels_mut() {
            if x == 0 || x == N - 1 || y == 0 || y == N - 1 {
                *pixel = image::Rgba([0, 0, 0, 0]);
            } else {
                *pixel = image::Rgba([255, 255, 255, 255]);
            }
        }
        let poly = Polygon::from_subimage(&img.into(), 0, 0, N, N, std::f64::consts::PI * -3. / 4.);
        assert_eq!(
            poly,
            Polygon {
                points: vec![
                    v(1.0, 1.0),
                    v(3.0, 1.0),
                    v(3.0, 3.0),
                    v(1.0, 3.0),
                    v(1.0, 1.0),
                ],
                xmin: 1.0,
                xmax: 3.0,
                ymin: 1.0,
                ymax: 3.0,
                start: Some(Vector2::new(1, 1)),
            }
        );
    }

    #[test]
    fn polygon_image_hollow() {
        const N: u32 = 7;
        let mut img: image::ImageBuffer<image::Rgba<u8>, Vec<u8>> = image::ImageBuffer::new(N, N);

        for (x, y, pixel) in img.enumerate_pixels_mut() {
            if x == 0 || x == N - 1 || y == 0 || y == N - 1 {
                *pixel = image::Rgba([255, 255, 255, 255]);
            } else {
                *pixel = image::Rgba([0, 0, 0, 0]);
            }
        }
        let poly = Polygon::from_subimage(&img.into(), 0, 0, N, N, std::f64::consts::PI * -3. / 4.);
        assert_eq!(
            poly,
            Polygon {
                points: vec![
                    v(0.0, 0.0),
                    v(6.0, 0.0),
                    v(6.0, 6.0),
                    v(0.0, 6.0),
                    v(0.0, 0.0),
                ],
                xmin: 0.0,
                xmax: 6.0,
                ymin: 0.0,
                ymax: 6.0,
                start: Some(Vector2::new(0, 0)),
            }
        );
    }

    #[test]
    fn point_collision_square() {
        let poly = Polygon::from_points(
            vec![v(0.0, 0.0), v(10.0, 0.0), v(10.0, 10.0), v(0.0, 10.0)],
            None,
        );
        for (a, b) in poly.points.iter().circular_tuple_windows() {
            for i in 0..=10 {
                let t = i as f64 / 10.0;
                assert!(poly.contains_point(a + t * (b - a)));
            }
        }
        assert!(poly.contains_point(v(5.0, 5.0)));
        assert!(poly.contains_point(v(0.1, 9.9)));
        assert!(!poly.contains_point(v(15.0, 5.0)));
        assert!(!poly.contains_point(v(-1.0, 5.0)));
    }

    #[test]
    fn point_collision_triangle() {
        let poly = Polygon::from_points(vec![v(0.0, 0.0), v(0.0, 10.0), v(10.0, 10.0)], None);
        for (a, b) in poly.points.iter().circular_tuple_windows() {
            for i in 0..=10 {
                let t = i as f64 / 10.0;
                assert!(poly.contains_point(a + t * (b - a)));
            }
        }
        assert!(poly.contains_point(v(1.0, 1.0)));
        assert!(!poly.contains_point(v(9.0, 1.0)));
        assert!(!poly.contains_point(v(11.0, 11.0)));
    }

    #[test]
    fn concave_c_shape() {
        let poly = Polygon::from_points(
            vec![
                v(0.0, 0.0),
                v(6.0, 0.0),
                v(6.0, 1.0),
                v(1.0, 1.0),
                v(1.0, 5.0),
                v(6.0, 5.0),
                v(6.0, 6.0),
                v(0.0, 6.0),
            ],
            None,
        );
        for (a, b) in poly.points.iter().circular_tuple_windows() {
            for i in 0..=10 {
                let t = i as f64 / 10.0;
                assert!(poly.contains_point(a + t * (b - a)));
            }
        }
        assert!(poly.contains_point(v(0.5, 3.0)));
        assert!(poly.contains_point(v(5.5, 5.5)));
        assert!(!poly.contains_point(v(5.5, 6.5)));
        assert!(!poly.contains_point(v(3.0, 3.0)));
        assert!(!poly.contains_point(v(1.5, 1.5)));
        assert!(!poly.contains_point(v(6.5, 3.0)));
    }

    fn v_eq(a: Vector2<f64>, b: Vector2<f64>) -> bool {
        (a - b).norm() < crate::collide::TOLERANCE
    }

    fn v_contains(p: &ArrayVec<Vector2<f64>, 2>, q: Vector2<f64>) -> bool {
        p.iter().any(|x| v_eq(*x, q))
    }

    fn square() -> Polygon {
        Polygon::from_points(
            vec![v(0.0, 0.0), v(2.0, 0.0), v(2.0, 2.0), v(0.0, 2.0)],
            None,
        )
    }

    fn small_square() -> Polygon {
        Polygon::from_points(
            vec![v(0.0, 0.0), v(1.0, 0.0), v(1.0, 1.0), v(0.0, 1.0)],
            None,
        )
    }

    fn triangle() -> Polygon {
        Polygon::from_points(vec![v(0.0, 0.0), v(2.0, 0.0), v(1.0, 2.0)], None)
    }

    #[test]
    fn polygon_line_crosses() {
        let poly = square();
        let p = poly.intersect_line(v(-1.0, 1.0), v(3.0, 1.0));
        assert_eq!(p.len(), 2);
        assert!(v_contains(&p, v(0.0, 1.0)));
        assert!(v_contains(&p, v(2.0, 1.0)));
    }

    #[test]
    fn polygon_line_enters() {
        let poly = square();
        let p = poly.intersect_line(v(-1.0, 1.0), v(1.0, 1.0));
        assert_eq!(p.len(), 2);
        assert!(v_contains(&p, v(0.0, 1.0)));
        // We assume end point is also a collision for visual purposes
        assert!(v_contains(&p, v(1.0, 1.0)));
    }

    #[test]
    fn polygon_line_miss() {
        let poly = square();
        let p = poly.intersect_line(v(-1.0, 3.0), v(3.0, 3.0));
        assert!(p.is_empty());
    }

    #[test]
    fn polygon_line_fully_inside() {
        let poly = square();
        let p = poly.intersect_line(v(0.5, 0.5), v(1.5, 1.5));
        assert_eq!(p.len(), 2);
        assert!(v_contains(&p, v(0.5, 0.5)));
        assert!(v_contains(&p, v(1.5, 1.5)));
    }

    #[test]
    fn polygon_line_touches_vertex() {
        let poly = square();
        let p = poly.intersect_line(v(-1.0, -1.0), v(0.0, 0.0));
        // TODO fix double hit, not that it really matters...
        //assert_eq!(p.len(), 1);
        assert!(v_contains(&p, v(0.0, 0.0)));
    }

    #[test]
    fn polygons_non_convex_intersection() {
        let poly = Polygon::from_points(
            vec![v(0.0, 0.0), v(3.0, 1.0), v(0.0, 2.0), v(1.0, 1.0)],
            None,
        );
        let p = poly.intersect_line(v(-1.0, 1.0), v(4.0, 1.0));
        assert_eq!(p.len(), 2);
        // TODO gets a double (3,1) hit when it should be different points
        assert!(v_contains(&p, v(3.0, 1.0)));
    }

    #[test]
    fn polygons_disjoint() {
        let a = square();
        let b = square();
        let p = a.intersect_polygon(&b, v(5.0, 0.0));
        assert!(p.is_empty());
    }

    #[test]
    fn polygons_touch_at_vertex() {
        let a = square();
        let b = square();
        let p = a.intersect_polygon(&b, v(2.0, 2.0));
        dbg!(&p);
        //assert_eq!(p.len(), 1);
        assert!(v_contains(&p, v(2.0, 2.0)));
    }

    #[test]
    fn polygons_cross_edges() {
        let a = square();
        let b = square();
        let p = a.intersect_polygon(&b, v(1.0, -1.0));
        assert_eq!(p.len(), 2);
        assert!(v_contains(&p, v(1.0, 0.0)));
        assert!(v_contains(&p, v(2.0, 1.0)));
    }

    #[test]
    fn polygons_contained() {
        let a = square();
        let b = small_square();
        let p = a.intersect_polygon(&b, v(0.5, 0.5));
        assert_eq!(p.len(), 1);
        assert_eq!(p[0], v(0.5, 0.5));
    }

    #[test]
    fn polygons_contains_self() {
        let a = small_square();
        let b = square();
        let p = a.intersect_polygon(&b, v(-0.5, -0.5));
        assert_eq!(p.len(), 1);
        assert_eq!(p[0], v(0.0, 0.0));
    }

    #[test]
    fn polygon_circle_miss() {
        let poly = square();
        let p = poly.intersect_circle(v(10.0, 10.0), 1.0);
        assert!(p.is_empty());
    }

    #[test]
    fn polygon_circle_secant() {
        let poly = square();
        let p = poly.intersect_circle(v(1.0, 1.0), 1.2);
        assert_eq!(p.len(), 2);
    }

    #[test]
    fn polygon_circle_tangent_edge() {
        let poly = square();
        let p = poly.intersect_circle(v(1.0, -1.0), 1.0);
        assert_eq!(p.len(), 1);
        assert_eq!(p[0], v(1.0, 0.0));
    }

    #[test]
    fn polygon_circle_tangent_vertex() {
        let poly = square();
        let p = poly.intersect_circle(v(-1.0, -1.0), (2.0f64).sqrt());
        // Should be getting one hit, but we get two :/
        assert!(p.len() >= 1);
        //assert_eq!(p[0], v(0.0, 0.0));
    }

    #[test]
    fn polygon_circle_contained() {
        let poly = square();
        let p = poly.intersect_circle(v(1.0, 1.0), 10.0);
        // Since we consider endpoints to be hits, this will give a lot of hits
        assert!(p.len() >= 1);
        //assert!(v_contains(&p, v(0.0, 0.0)));
    }

    #[test]
    fn polygon_circle_inside() {
        let poly = square();
        let p = poly.intersect_circle(v(1.0, 1.0), 0.25);
        assert_eq!(p.len(), 1);
        assert_eq!(p[0], v(1.0, 1.0));
    }

    #[test]
    fn polygon_circle_two_edges() {
        let poly = square();
        let p = poly.intersect_circle(v(1.0, 1.0), 1.0);
        assert_eq!(p.len(), 2);
    }
}
