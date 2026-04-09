#![allow(unused)]
use nalgebra::Vector2;

/// Represents a solid that can move and have collisions
pub struct Solid {
   /// Mass of the solid
   mass: f64,
   /// Direction the solid is facing
   dir: f64,
   /// Angular velocity
   dir_vel: f64,

   /// Current position
   pos: Vector2<f64>,
   /// Current velocity
   vel: Vector2<f64>,
   /// Previous position (used for collisions)
   pre: Vector2<f64>,

   /// Relative acceleration on the X-axis
   accel: f64,

   /// Maximum *drift* speed
   speed_max: f64,
   /// A factor that *divides* the over-the-max opposing force
   aerodynamics: f64,

   /// Whether to use high precision approximation
   precise: bool,
}
