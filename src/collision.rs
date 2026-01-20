use anyhow::Result;
use image::GenericImageView;
use nalgebra::{Point2, Vector2};
use serde::{Deserialize, Serialize};

const ALPHA_THRESHOLD: u8 = 50;

#[derive(Debug, PartialEq, Serialize, Deserialize)]
pub struct Polygon {
    points: Vec<Point2<f32>>,
    xmin: f32,
    xmax: f32,
    ymin: f32,
    ymax: f32,
}

impl Polygon {
    fn from_binary(points: &[bool], width: i32, height: i32) -> Self {
        let c = trace_contour(&points, width, height);
        let c: Vec<mint::Point2<i32>> = c
            .iter()
            .map(|v| mint::Point2::<i32> { x: v.x, y: v.y })
            .collect();
        let idx = ramer_douglas_peucker::rdp(&c, 0.5);
        let points: Vec<Point2<f32>> = idx
            .iter()
            .map(|v| {
                let m = c[*v];
                Point2::new(m.x as f32, m.y as f32)
            })
            .collect();

        let mut xmin = f32::INFINITY;
        let mut xmax = -f32::INFINITY;
        let mut ymin = f32::INFINITY;
        let mut ymax = -f32::INFINITY;
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
        }
    }

    fn from_subimage(img: &image::DynamicImage, x: u32, y: u32, w: u32, h: u32) -> Self {
        let mut points: Vec<bool> = Vec::with_capacity((w * h) as usize);
        for u in x..x + w {
            for v in y..y + h {
                points.push(img.get_pixel(u, v)[3] > ALPHA_THRESHOLD);
            }
        }
        Self::from_binary(&points, w as i32, h as i32)
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
fn trace_contour(image: &[bool], width: i32, height: i32) -> Vec<Point2<i32>> {
    // Find starting pixel (top-left foreground)
    let mut start = None;
    'outer: for y in 0..height {
        for x in 0..width {
            if image[(x * height + y) as usize] {
                start = Some(Point2::new(x, y));
                break 'outer;
            }
        }
    }
    let start = match start {
        Some(p) => p,
        None => return vec![],
    };

    let mut contour = Vec::new();
    let mut current = start;
    let mut dir = 4; // Start by going right

    loop {
        contour.push(current);

        let mut found_next = false;

        // Search neighbors clockwise starting from (dir + 1)
        for i in 0..8 {
            let ndir = (dir + 1 + i) % 8;
            let d = DIRS[ndir];
            let n = current + d;

            if n.x >= 0 && n.y >= 0 && n.x < width && n.y < height {
                if image[(n.x * height + n.y) as usize] {
                    current = n;
                    dir = (ndir + 4) % 8; // opposite direction
                    found_next = true;
                    break;
                }
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
    contour
}
