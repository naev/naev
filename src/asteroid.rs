#![allow(dead_code)]
use crate::commodity::CommodityRef;
use collide::polygon::SpinPolygon;
use renderer::texture::Texture;
use std::sync::Arc;

struct Material {
   material: CommodityRef,
   quantity: i32,
   rarity: i32,
}

enum Gfx {
   Single(Texture),
   Sprite(Texture),
}

pub struct Type {
   name: String,
   scanned_msg: String,
   gfx: Vec<Gfx>,
   material: Vec<Material>,
   armour_min: f64,
   armour_max: f64,
   absorb: f64,
   damage: f64,
   disable: f64,
   penetration: f64,
   exp_radius: f64,
   alert_range: f64,
}

pub struct TypeGroup {
   name: String,
   types: Vec<(Arc<Type>, f64)>,
   wtotal: f64,
}

enum State {
   Xx,
   XxToBg,
   Xb,
   BgToFg,
   Fg,
   FgToBg,
   Bx,
   BgToXx,
}

impl State {
   fn from_c(c: naevc::AsteroidState) -> Self {
      match c {
         naevc::AsteroidState_ASTEROID_XX => State::Xx,
         naevc::AsteroidState_ASTEROID_XX_TO_BG => State::XxToBg,
         naevc::AsteroidState_ASTEROID_XB => State::Xb,
         naevc::AsteroidState_ASTEROID_BG_TO_FG => State::BgToFg,
         naevc::AsteroidState_ASTEROID_FG => State::Fg,
         naevc::AsteroidState_ASTEROID_FG_TO_BG => State::FgToBg,
         naevc::AsteroidState_ASTEROID_BX => State::Bx,
         naevc::AsteroidState_ASTEROID_BG_TO_XX => State::BgToXx,
         _ => State::Xx,
      }
   }

   fn to_c(&self) -> naevc::AsteroidState {
      match self {
         State::Xx => naevc::AsteroidState_ASTEROID_XX,
         State::XxToBg => naevc::AsteroidState_ASTEROID_XX,
         State::Xb => naevc::AsteroidState_ASTEROID_XX,
         State::BgToFg => naevc::AsteroidState_ASTEROID_XX,
         State::Fg => naevc::AsteroidState_ASTEROID_XX,
         State::FgToBg => naevc::AsteroidState_ASTEROID_XX,
         State::Bx => naevc::AsteroidState_ASTEROID_XX,
         State::BgToXx => naevc::AsteroidState_ASTEROID_XX,
      }
   }
}

pub struct Asteroid {
   id: i32,
   parent: i32,
   state: State,
   atype: Arc<Type>,
   gfx: usize,
   poly: SpinPolygon,
   armour: f64,

   sol: naevc::Solid,
   ang: f64,
   spin: f64,

   timer: f64,
   timer_max: f64,
   scan_alpha: f64,
   scanned: bool,
}
