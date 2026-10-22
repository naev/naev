#![allow(dead_code, unused)]
use crate::array;
use crate::array::Array;
use crate::commodity::CommodityRef;
use crate::pilot;
use crate::rng;
use crate::rng::{range, rng};
use anyhow::Context as AnyhowContext;
use anyhow::Result;
use audio::{AudioBuilder, AudioType};
use bvh_arena::{Bvh, volumes::Aabb};
use collide::polygon::Polygon;
use collide::polygon::SpinPolygon;
use helpers::ReferenceC;
use mlua::{BorrowedStr, Either, FromLua, MetaMethod, UserData, UserDataMethods, UserDataRef};
use naev_core::{nxml, nxml_err_attr_missing, nxml_warn_node_unknown};
use nalgebra::Vector2;
use nlog::{debugx, warn, warn_err};
use physics::vec2::Vec2;
use rayon::prelude::*;
use renderer::texture::{Texture, TextureBuilder};
use renderer::{Context, ContextWrapper, camera, camera::Coords, colour};
use slotmap::{Key, KeyData, SlotMap};
use std::collections::HashMap;
use std::ffi::{CStr, CString, OsStr, c_char, c_int, c_void};
use std::mem::MaybeUninit;
use std::path::{Path, PathBuf};
use std::sync::{Arc, LazyLock, Mutex, RwLock};
use tracing::instrument;

/// Buffer for appearance of debris
const DEBRIS_BUFFER: f32 = 1000.0;
/// How long it takes to fade in and out the scanning text
const SCAN_FADE: f64 = 10.0;

static DEBRIS_GFX: LazyLock<RwLock<Vec<Arc<GfxType>>>> = LazyLock::new(Default::default);
static DEBRIS: LazyLock<Mutex<Vec<Debris>>> = LazyLock::new(Default::default);

fn get_inner(field: &naevc::AsteroidAnchor) -> &AnchorInner {
   unsafe { &*(field.inner as *const AnchorInner) }
}

fn get_inner_mut(field: &naevc::AsteroidAnchor) -> &mut AnchorInner {
   unsafe { &mut *(field.inner as *mut AnchorInner) }
}

#[derive(Debug)]
struct Material {
   material: CommodityRef,
   quantity: i32,
   rarity: i32,
}

#[derive(Debug)]
struct Gfx2d {
   texture: Texture,
   poly: SpinPolygon,
}

#[derive(Debug)]
enum GfxType {
   Single(Gfx2d),
   Sprite(Gfx2d),
}

impl GfxType {
   pub fn radius(&self) -> f32 {
      match self {
         GfxType::Single(gfx) => gfx.texture.sw,
         GfxType::Sprite(gfx) => gfx.texture.sw,
      }
   }

   pub fn draw(&self, ctx: &Context, pos: Coords, angle: f32, col: colour::Colour) {
      match self {
         GfxType::Single(gfx) => {
            let tex = &gfx.texture;
            tex.draw_sprite_scale_rotate(ctx, pos, 1.0, angle, 0, 0, col);
         }
         GfxType::Sprite(gfx) => {
            let tex = &gfx.texture;
            let (sx, sy) = tex.sprite_from_dir(angle);
            tex.draw_sprite(ctx, pos, sx, sy, col);
         }
      };
   }
}

struct Debris {
   gfx: Arc<GfxType>,
   pos: Vector2<f32>,
   vel: Vector2<f32>,
   angle: f32,
   /// Height with respect to the player
   height: f32,
   alpha: f32,
}

impl Debris {
   pub fn new() -> Self {
      let gfx = {
         let debris_gfx = &DEBRIS_GFX.read().unwrap();
         debris_gfx[rng::range(0..debris_gfx.len())].clone()
      };
      let (screen_w, screen_h) = {
         let dims = renderer::Context::get().dimensions.read().unwrap();
         (dims.view_width, dims.view_height)
      };
      let x = -DEBRIS_BUFFER + rng::<f32>() * (screen_w + 2.0 * DEBRIS_BUFFER);
      let y = -DEBRIS_BUFFER + rng::<f32>() * (screen_h + 2.0 * DEBRIS_BUFFER);
      let pos = Vector2::new(x, y);
      let theta = std::f32::consts::TAU * rng::<f32>();
      let val = 20.0 * rng::<f32>();
      let vel = Vector2::new(val * theta.cos(), val * theta.sin());
      Debris {
         gfx,
         pos,
         vel,
         angle: rng::<f32>() * std::f32::consts::TAU,
         height: 0.8 + rng::<f32>() * 0.4,
         alpha: 0.0,
      }
   }

   fn update(
      &mut self,
      dt: f32,
      dpos: Vector2<f32>,
      screen_w: f32,
      screen_h: f32,
      cam: &camera::Camera,
   ) {
      self.pos += self.vel * dt - dpos.cast::<f32>();

      if self.pos.x > screen_w + DEBRIS_BUFFER {
         self.pos.x -= screen_w + 2.0 * DEBRIS_BUFFER;
      } else if self.pos.x < -DEBRIS_BUFFER {
         self.pos.x += screen_w + 2.0 * DEBRIS_BUFFER;
      }
      if self.pos.y > screen_h + DEBRIS_BUFFER {
         self.pos.y -= screen_h + 2.0 * DEBRIS_BUFFER;
      } else if self.pos.y < -DEBRIS_BUFFER {
         self.pos.y += screen_h + 2.0 * DEBRIS_BUFFER;
      }

      // Set alpha based on position
      // TODO there seems to be some offset mistake or something going on here, not too big of
      // an issue though
      let s = cam.screen_to_game_coords(self.pos).cast::<f64>();
      if infield(s).is_some() {
         self.alpha = (self.alpha + 0.5 * dt).min(1.0);
      } else {
         self.alpha = (self.alpha - 0.5 * dt).max(0.0);
      }
   }

   fn render(&self, ctx: &Context) {
      let col = colour::Colour::new_alpha(1.0, 1.0, 1.0, self.alpha);
      self
         .gfx
         .draw(ctx, Coords::Screen(self.pos), self.angle, col);
   }
}

#[derive(Default, Debug)]
pub struct Type {
   name: String,
   scanned_msg: String,
   gfx: Vec<Arc<GfxType>>,
   material: Vec<Material>,
   armour_min: f64,
   armour_max: f64,
   absorb: f64,
   damage: f64,
   disable: f64,
   knockback: f64,
   penetration: f64,
   exp_radius: f64,
   alert_range: f64,
   scanned_msg_c: CString,
}

impl Type {
   fn get(name: &str) -> Option<Arc<Self>> {
      TYPES.get(name).cloned()
   }

   fn get_r(name: &str) -> Result<Arc<Self>> {
      Type::get(name).with_context(|| format!("Asteroid Type '{name}' not found"))
   }

   fn load_xml<P: AsRef<Path>>(ctx: &ContextWrapper, filename: P) -> Result<Self> {
      let data = ndata::read(filename)?;
      let doc = roxmltree::Document::parse(std::str::from_utf8(&data)?)?;
      let root = doc.root_element();
      let name = String::from(match root.attribute("name") {
         Some(n) => n,
         None => {
            return nxml_err_attr_missing!("Asteroid Type", "name");
         }
      });
      let mut at = Type {
         name,
         damage: 100.0,
         penetration: naevc::FULL_PENETRATION,
         exp_radius: 50.0,
         alert_range: 7000.0,
         ..Default::default()
      };
      for node in root.children() {
         if !node.is_element() {
            continue;
         }
         match node.tag_name().name().to_lowercase().as_str() {
            "scanned" => at.scanned_msg = nxml::node_string(node)?,
            "armour_min" => at.armour_min = nxml::node_f64(node)?,
            "armour_max" => at.armour_max = nxml::node_f64(node)?,
            "absorb" => at.absorb = nxml::node_f64(node)?,
            "damage" => at.damage = nxml::node_f64(node)?,
            "disable" => at.disable = nxml::node_f64(node)?,
            "knockback" => at.knockback = nxml::node_f64(node)?,
            "penetration" => at.penetration = nxml::node_f64(node)?,
            "exp_radius" => at.exp_radius = nxml::node_f64(node)?,
            "alert_range" => at.alert_range = nxml::node_f64(node)?,
            "gfx" => {
               let path = nxml::node_texturepath(node, "gfx/spob/space/asteroid/")?;
               let texture = TextureBuilder::new().path(&path).build_wrap(ctx)?;
               let poly = SpinPolygon::from_image_path(&path, 1, 1)?;
               let gfx = Gfx2d { texture, poly };
               at.gfx.push(GfxType::Single(gfx).into())
            }
            "commodity" => {
               let mut material = None;
               let mut quantity = None;
               let mut rarity = 0;
               for node in node.children() {
                  if !node.is_element() {
                     continue;
                  }
                  match node.tag_name().name().to_lowercase().as_str() {
                     "name" => material = Some(CommodityRef::new_r(nxml::node_str(node)?)?),
                     "quantity" => quantity = Some(nxml::node_str(node)?.parse()?),
                     "rarity" => rarity = nxml::node_str(node)?.parse()?,
                     tag => nxml_warn_node_unknown!("Asteroid Type Commodity", &at.name, tag),
                  }
               }
               if let Some(material) = material
                  && let Some(quantity) = quantity
               {
                  at.material.push(Material {
                     material,
                     quantity,
                     rarity,
                  });
               } else {
                  warn!(
                     "Asteroid Type '{}' has not fully defined commodity.",
                     at.name
                  );
               }
            }
            tag => nxml_warn_node_unknown!("Asteroid Type", &at.name, tag),
         }
      }
      at.scanned_msg_c = CString::new(at.scanned_msg.clone())?;
      Ok(at)
   }

   #[instrument(skip(ctx))]
   fn load<P: AsRef<Path> + std::fmt::Debug>(
      ctx: &ContextWrapper,
      filename: P,
   ) -> Option<Result<Self>> {
      let at = {
         let ext = filename.as_ref().extension();
         if ext == Some(OsStr::new("xml")) {
            Self::load_xml(ctx, &filename)
         } else {
            return None;
         }
      }
      .with_context(|| {
         format!(
            "unable to load Asteroid Type '{}'",
            filename.as_ref().display()
         )
      });
      Some(at)
   }
}

#[derive(Debug, Default)]
pub struct TypeGroup {
   name: String,
   types: Vec<(Arc<Type>, f64)>,
   wtotal: f64,
   // C stuff, remove when we can.
   cname: CString,
}

impl TypeGroup {
   fn load_xml<P: AsRef<Path>>(filename: P) -> Result<Self> {
      let data = ndata::read(filename)?;
      let doc = roxmltree::Document::parse(std::str::from_utf8(&data)?)?;
      let root = doc.root_element();
      let name = String::from(match root.attribute("name") {
         Some(n) => n,
         None => {
            return nxml_err_attr_missing!("Asteroid Group", "name");
         }
      });
      let mut group = TypeGroup {
         cname: CString::new(&*name)?,
         name,
         ..Default::default()
      };
      for node in root.children() {
         if !node.is_element() {
            continue;
         }
         match node.tag_name().name().to_lowercase().as_str() {
            "type" => {
               let atype = Type::get_r(nxml::node_str(node)?)?;
               let weight = match node.attribute("weight") {
                  Some(w) => w.parse::<f64>()?,
                  None => 1.0,
               };
               group.types.push((atype, weight));
            }
            tag => nxml_warn_node_unknown!("Asteroid Group", &group.name, tag),
         }
      }
      for (_, weight) in &group.types {
         group.wtotal += weight;
      }
      Ok(group)
   }

   #[instrument]
   fn load<P: AsRef<Path> + std::fmt::Debug>(filename: P) -> Option<Result<Self>> {
      let at = {
         let ext = filename.as_ref().extension();
         if ext == Some(OsStr::new("xml")) {
            Self::load_xml(&filename)
         } else {
            return None;
         }
      }
      .with_context(|| {
         format!(
            "unable to load Asteroid Group '{}'",
            filename.as_ref().display()
         )
      });
      Some(at)
   }
}

#[derive(Debug, Default, Copy, Clone, PartialEq)]
enum State {
   #[default]
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
   fn from_ffi(c: naevc::AsteroidState) -> Self {
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

   fn to_ffi(self) -> naevc::AsteroidState {
      match self {
         State::Xx => naevc::AsteroidState_ASTEROID_XX,
         State::XxToBg => naevc::AsteroidState_ASTEROID_XX_TO_BG,
         State::Xb => naevc::AsteroidState_ASTEROID_XB,
         State::BgToFg => naevc::AsteroidState_ASTEROID_BG_TO_FG,
         State::Fg => naevc::AsteroidState_ASTEROID_FG,
         State::FgToBg => naevc::AsteroidState_ASTEROID_FG_TO_BG,
         State::Bx => naevc::AsteroidState_ASTEROID_BX,
         State::BgToXx => naevc::AsteroidState_ASTEROID_BG_TO_XX,
      }
   }

   fn next(s: Self) -> Self {
      use State::*;
      match s {
         Xx => XxToBg,
         XxToBg => Xb,
         Xb => BgToFg,
         BgToFg => Fg,
         Fg => FgToBg,
         FgToBg => Bx,
         Bx => BgToXx,
         BgToXx => Xx,
      }
   }

   fn prev(s: Self) -> Self {
      use State::*;
      match s {
         XxToBg => Xx,
         Xb => XxToBg,
         BgToFg => Xb,
         Fg => BgToFg,
         FgToBg => Fg,
         Bx => FgToBg,
         BgToXx => Bx,
         Xx => BgToXx,
      }
   }
}

#[derive(Debug)]
pub struct Asteroid {
   id: AsteroidRef,
   parent: usize,
   state: State,
   atype: Arc<Type>,
   gfx: Arc<GfxType>,
   armour: f64,

   solid: naevc::Solid,
   angle: f64,
   spin: f64,

   timer: f64,
   timer_max: f64,
   scan_alpha: f64,
   scanned: bool,
}

fn infield(p: Vector2<f64>) -> Option<usize> {
   if let Some(cur_system) = crate::system::cur() {
      for e in cur_system.astexclude() {
         if (p - Vector2::new(e.pos.x, e.pos.y)).norm_squared() <= e.radius * e.radius {
            return None;
         }
      }
      for (i, a) in cur_system.asteroids().iter().enumerate() {
         if (p - Vector2::new(a.pos.x, a.pos.y)).norm_squared() <= a.radius * a.radius {
            return Some(i);
         }
      }
   }
   None
}

impl Asteroid {
   fn try_new(field: &naevc::AsteroidAnchor, creating: bool) -> Option<Self> {
      let n = field.radius * rng::<f64>().sqrt();
      let a = std::f64::consts::TAU * rng::<f64>();

      // Try to find position
      let mut pos = Vector2::new(field.pos.x, field.pos.y);
      for i in 0..1000 {
         pos = Vector2::new(field.pos.x + n * a.cos(), field.pos.y + n * a.sin());
         let outfield = infield(pos).is_none();
         if creating && outfield {
            return None;
         } else if !outfield {
            break;
         }
      }

      // Choose asteroid type
      let r = field.groupswtotal * rng::<f64>();
      let mut wmax = 0.0;
      let groups = unsafe { array::array_as_slice(field.groups) };
      let groupsw = unsafe { array::array_as_slice(field.groupsw) };
      let mut atype = None;
      'outter: for (g, w) in groups.iter().zip(groupsw.iter()) {
         let g = unsafe { &*(*g as *mut TypeGroup) };
         wmax += w;
         if r > wmax {
            continue;
         }

         let mut wi = 0.0;
         let r = g.wtotal * rng::<f64>();
         for (t, w) in g.types.iter() {
            wi += w;
            if r > wi {
               continue;
            }
            atype = Some(t);
            break 'outter;
         }
      }

      if let Some(atype) = atype {
         let armour = range(atype.armour_min..=atype.armour_max);
         let theta = std::f64::consts::TAU * rng::<f64>();
         let vmod = field.maxspeed * rng::<f64>();
         let vel = Vector2::new(vmod * theta.cos(), vmod * theta.sin());
         let solid = {
            let mut solid = MaybeUninit::<naevc::Solid>::uninit();
            unsafe {
               naevc::solid_init(
                  solid.as_mut_ptr(),
                  0.0,
                  0.0,
                  pos.as_ptr() as *const naevc::vec2,
                  vel.as_ptr() as *const naevc::vec2,
                  naevc::SOLID_UPDATE_EULER as i32,
               );
            }
            unsafe { solid.assume_init() }
         };
         let spin = (1.0 - 2.0 * rng::<f64>()) * field.maxspin;
         let gfx = atype.gfx[range(0..atype.gfx.len())].clone();

         Some(Self {
            id: AsteroidRef::null(),
            parent: field.id.try_into().unwrap_or_else(|e| {
               warn_err!(e);
               0
            }),
            state: State::Xx,
            atype: atype.clone(),
            gfx,
            armour,

            solid,

            angle: std::f64::consts::TAU * rng::<f64>(),
            spin,
            timer: -1.0,
            timer_max: -1.0,
            scan_alpha: 0.0,
            scanned: false,
         })
      } else {
         None
      }
   }

   fn as_lua(&self) -> LuaAsteroid {
      LuaAsteroid {
         parent: self.parent,
         id: self.id,
      }
   }

   fn pos(&self) -> Vector2<f64> {
      Vector2::new(self.solid.pos.x, self.solid.pos.y)
   }

   fn vel(&self) -> Vector2<f64> {
      Vector2::new(self.solid.vel.x, self.solid.vel.y)
   }

   fn aabb(&self) -> Aabb<2> {
      let (x, y) = (self.solid.pos.x as f32, self.solid.pos.y as f32);
      let r = self.gfx.radius();
      Aabb::from_min_max(Vector2::new(x - r, y - r), Vector2::new(x + r, y + r))
   }

   /// Updates a single asteroid taking into account keeping it in the anchor
   fn update(
      &mut self,
      dt: f64,
      anchor: &naevc::AsteroidAnchor,
      astex: Option<&[naevc::AsteroidExclusion]>,
   ) -> bool {
      let inner = get_inner(anchor);
      let off = Vector2::new(anchor.pos.x, anchor.pos.y) - self.pos();
      let d2 = off.norm_squared();
      let setvel = if d2 >= anchor.radius * anchor.radius {
         let d = d2.sqrt();
         self.solid.vel.x += anchor.accel * dt * off.x / d;
         self.solid.vel.y += anchor.accel * dt * off.y / d;
         true
      } else if let Some(astex) = astex {
         let mut setvel = false;
         for ex in astex {
            if ex.affects == 0 {
               continue;
            }

            let eoff = self.pos() - Vector2::new(anchor.pos.x, anchor.pos.y);
            let ed2 = eoff.norm_squared();
            if ed2 <= ex.radius * ex.radius {
               let ed = ed2.sqrt();
               self.solid.vel.x += anchor.accel * dt * eoff.x / ed;
               self.solid.vel.y += anchor.accel * dt * eoff.y / ed;
               setvel = true;
            }
         }
         setvel
      } else {
         false
      };

      if setvel {
         // Enforce max speed
         let speed = self.vel().norm();
         if speed > anchor.maxspeed {
            self.solid.vel.x *= anchor.maxspeed / speed;
            self.solid.vel.y *= anchor.maxspeed / speed;
         }
      }

      // TODO use physics stuff
      self.solid.pre = self.solid.pos;
      self.solid.pos.x += self.solid.vel.x * dt;
      self.solid.pos.y += self.solid.vel.y * dt;

      // Update angle
      self.angle += self.spin * dt;

      // Figure out if state change is applicable
      let forced = self.timer < 0.; // Forced by Lua or whatever
      self.timer -= dt;
      if self.timer < 0. {
         match self.state {
            State::Fg => {
               if !forced
                  && let Some(player) = pilot::player()
                  && ((player.pos() - self.pos()).norm_squared() < 1500.0 * 1500.0
                     || unsafe {
                        (player.0.as_ref().nav_anchor == self.parent as i32
                           && player.0.as_ref().nav_asteroid == self.id.as_ffi())
                     })
               {
                  // Lower state so it gets incremented
                  self.state = State::prev(self.state);
               } else {
                  unsafe {
                     naevc::pilot_untargetAsteroid(self.parent as i32, self.id.as_ffi());
                  }
               }
               self.timer_max = 1.0 + 3.0 * rng::rng::<f64>();
               self.timer = self.timer_max;
            }
            State::Xb | State::Bx | State::XxToBg => {
               self.timer_max = 1.0 + 3.0 * rng::rng::<f64>();
               self.timer = self.timer_max;
            }
            State::FgToBg => {
               self.timer_max = 10.0 + 20.0 * rng::rng::<f64>();
               self.timer = self.timer_max;
            }
            State::BgToFg => {
               self.timer_max = 90.0 + 30.0 * rng::rng::<f64>();
               self.timer = self.timer_max;
            }
            State::BgToXx => {
               // Remove asteroid, try to respawn later
               return false;
            }
            State::Xx => (),
         }
         self.state = State::next(self.state);
      }

      if self.scanned {
         if self.state == State::Fg {
            self.scan_alpha += SCAN_FADE * dt;
            self.scan_alpha.min(1.0);
         } else {
            self.scan_alpha -= SCAN_FADE * dt;
            self.scan_alpha.max(0.0);
         }
      }
      true
   }

   /// Renders a single asteroid onscreen
   fn render(&self, ctx: &Context) {
      if self.state == State::Xx {
         return;
      }

      let progress = (self.timer / self.timer_max) as f32;
      // colour::GREY20
      const DARK_R: f32 = 0.2;
      const DARK_G: f32 = 0.2;
      const DARK_B: f32 = 0.2;
      const DARK_COL: colour::Colour = colour::Colour::new(DARK_R, DARK_G, DARK_B);
      let col = match self.state {
         State::XxToBg => colour::Colour::new_alpha(DARK_R, DARK_G, DARK_B, 1.0 - progress),
         State::Xb | State::Bx => colour::Colour::new(DARK_R, DARK_G, DARK_B),
         State::BgToFg => DARK_COL.blend(&colour::WHITE, progress),
         State::Fg => colour::WHITE,
         State::FgToBg => colour::WHITE.blend(&DARK_COL, progress),
         State::BgToXx => colour::Colour::new_alpha(DARK_R, DARK_G, DARK_B, progress),
         State::Xx => unreachable!(),
      };

      self.gfx.draw(
         ctx,
         Coords::Game(Vector2::new(self.solid.pos.x, self.solid.pos.y).cast::<f32>()),
         self.angle as f32,
         col,
      );

      // Display scanned message text
      if self.scanned {
         let uv = ctx
            .game_to_screen_coords(Vector2::new(self.solid.pos.x, self.solid.pos.y).cast::<f32>())
            .cast::<f64>();
         let x = uv.x + self.gfx.radius() as f64 * 0.5;
         let y = uv.y - unsafe { naevc::gl_smallFont.h as f64 } * 0.5;
         let mut col = unsafe { naevc::cFontWhite };
         col.a = self.scan_alpha as f32;
         unsafe {
            naevc::gl_printRaw(
               &raw const naevc::gl_smallFont,
               x,
               y,
               &col,
               -1.,
               naevc::gettext_rust(self.atype.scanned_msg_c.as_ptr()),
            );
         }
      }
   }

   pub fn explode(&mut self, max_rarity: i32, mine_bonus: f64) {
      let at = &self.atype;

      static DTYPE_SPLASH: LazyLock<c_int> =
         LazyLock::new(|| crate::damagetype::dtype_get(c"explosion_splash".as_ptr()));
      let dmg = naevc::Damage {
         type_: *DTYPE_SPLASH,
         damage: at.damage,
         penetration: at.penetration,
         disable: at.disable,
         knockback: at.knockback,
      };
      unsafe {
         naevc::expl_explode(
            self.solid.pos.x,
            self.solid.pos.y,
            self.solid.vel.x,
            self.solid.vel.y,
            at.exp_radius,
            &dmg,
            std::ptr::null(),
            naevc::EXPL_MODE_SHIP as i32,
         );
      }

      // Explosion sound. TODO not hardcode
      match audio::Buffer::get_or_try_load(format!("snd/sounds/explosion{}", range(0..=2))) {
         Ok(sound) => {
            AudioBuilder::new(AudioType::Static)
               .buffer(sound.clone())
               .position(Some(self.pos().cast()))
               .velocity(Some(self.pos().cast()))
               .play(true)
               .build();
         }
         Err(e) => warn_err!(e),
      }

      let rad2 = at.alert_range * at.alert_range;
      let la = self.as_lua();
      unsafe {
         lua_pushasteroid(naevc::naevL as *mut mlua::lua_State, la.into_c());
      }
      for p in pilot::get_all() {
         if (self.pos() - p.pos()).norm_squared() <= rad2 {
            unsafe {
               naevc::pilot_msg(std::ptr::null(), p.0.as_ptr(), c"asteroid".as_ptr(), -1);
            }
         }
      }
      //unsafe{ naevc::lua_pop(naevc::naevL, 1 ); }
      unsafe {
         naevc::lua_settop(naevc::naevL, -(1) - 1);
      } // #define of lua_pop

      // Do the drop
      if max_rarity >= 0 {
         let mut ndrops = 0;
         for m in at.material.iter() {
            if m.rarity <= max_rarity {
               ndrops += 1;
            }
         }
         if ndrops > 0 {
            let r = rng::<f32>();
            let prob = 1.0 / ndrops as f32;
            let mut accum = 0.0;
            for m in at.material.iter() {
               if m.rarity > max_rarity {
                  continue;
               }
               accum += prob;
               if r > accum {
                  continue;
               }

               let nb = range(0..(m.quantity as f64 * mine_bonus).round() as usize) / 3;
               for i in 0..nb {
                  let pos = self.pos()
                     + Vector2::new(30.0 * rng::<f64>() - 15.0, 30.0 * rng::<f64>() - 15.0);
                  let vel = self.vel()
                     + Vector2::new(20.0 * rng::<f64>() - 10.0, 20.0 * rng::<f64>() - 10.0);
                  let ttl = 50.0 + rng::<f64>() * 10.0;
                  let quantity = range(1..=4);
                  crate::gatherable::Gatherable::add(m.material, pos, vel, ttl, quantity, false);
               }
               break;
            }
         }
      }

      // Remove target
      unsafe {
         naevc::pilot_untargetAsteroid(self.parent as i32, self.id.as_ffi());
      }

      // Make it respawns
      self.state = State::BgToXx;
      self.timer = 0.0;
      self.timer_max = 0.0;
   }
}

slotmap::new_key_type! {
pub struct AsteroidRef;
}
impl AsteroidRef {
   fn from_ffi(value: i64) -> Self {
      Self(KeyData::from_ffi(value as u64))
   }
}

static TYPES: LazyLock<HashMap<String, Arc<Type>>> = LazyLock::new(load_types);
static GROUPS: LazyLock<HashMap<String, TypeGroup>> = LazyLock::new(load_groups);

fn load_types() -> HashMap<String, Arc<Type>> {
   let ctx = Context::get().as_safe_wrap();
   let base: PathBuf = "asteroids/types".into();
   let mut data: HashMap<_, _> = ndata::read_dir(&base)
      .unwrap_or_else(|e| {
         warn_err!(e);
         Vec::new()
      })
      .par_iter()
      .filter_map(|filename| match Type::load(&ctx, base.join(filename)) {
         Some(Ok(at)) => Some((at.name.clone(), Arc::new(at))),
         Some(Err(e)) => {
            warn_err!(e);
            None
         }
         None => None,
      })
      .collect();
   data
}

fn load_groups() -> HashMap<String, TypeGroup> {
   let base: PathBuf = "asteroids/groups".into();
   let mut data: HashMap<_, _> = ndata::read_dir(&base)
      .unwrap_or_else(|e| {
         warn_err!(e);
         Vec::new()
      })
      .par_iter()
      .filter_map(|filename| match TypeGroup::load(base.join(filename)) {
         Some(Ok(group)) => Some((group.name.clone(), group)),
         Some(Err(e)) => {
            warn_err!(e);
            None
         }
         None => None,
      })
      .collect();
   data
}

pub fn load() -> Result<()> {
   #[cfg(debug_assertions)]
   let start = std::time::Instant::now();

   let _ = LazyLock::force(&TYPES);
   let _ = LazyLock::force(&GROUPS);

   // Some debug
   #[cfg(debug_assertions)]
   {
      let n = TYPES.len();
      let elapsed = start.elapsed().as_secs_f32();
      debugx!(
         gettext::ngettext(
            "Loaded {} Asteroid in {:.3} s",
            "Loaded {} Asteroids in {:.3} s",
            n as u64
         ),
         n,
         elapsed
      );
   }
   #[cfg(not(debug_assertions))]
   {
      let n = TYPES.len();
      debugx!(
         gettext::ngettext("Loaded {} Asteroid", "Loaded {} Asteroids", n as u64),
         n
      );
   }

   Ok(())
}

#[instrument]
pub fn update(dt: f64) {
   if let Some(cur_system) = crate::system::cur() {
      for ast in cur_system.asteroids_mut() {
         let inner = get_inner_mut(ast);
         let mut has_exclusion = false;
         for ex in cur_system.astexclude_mut() {
            if Vector2::new(ast.pos.x - ex.pos.x, ast.pos.y - ex.pos.y).norm_squared()
               <= ast.radius * ast.radius
            {
               ex.affects = 1;
               has_exclusion = true;
            } else {
               ex.affects = 0;
            }
         }

         let astex = match has_exclusion {
            true => Some(cur_system.astexclude()),
            false => None,
         };
         let mut destroyed = 0;
         inner.asteroids.retain(|_, a| {
            // Skip inexistent asteroids
            if a.state == State::Xx {
               a.timer -= dt;
               if a.timer < 0. {
                  a.state = State::XxToBg;
                  a.timer_max = 1.0 + 3.0 * rng::rng::<f64>();
                  a.timer = a.timer_max;
               }
               return true;
            }
            let retain = a.update(dt, ast, astex);
            if !retain {
               destroyed += 1;
            }
            retain
         });
         // Respawn destroyed asteroids
         for _ in 0..destroyed {
            if let Some(mut a) = Asteroid::try_new(ast, false) {
               a.timer_max = 10.0 + 20.0 * rng::rng::<f64>();
               a.timer = a.timer_max;
            } else {
               warn!("failed to respawn asteroid");
            }
         }

         // Quadtree stuff
         inner.update_bvh();
      }

      // Update debris
      if unsafe { naevc::space_isSimulation() == 0 } {
         let cam = camera::CAMERA.read().unwrap();
         let dpos = cam.der.cast::<f32>();
         let (screen_w, screen_h) = {
            let dims = renderer::Context::get().dimensions.read().unwrap();
            (dims.view_width, dims.view_height)
         };
         let dt = dt as f32;
         for d in DEBRIS.lock().unwrap().iter_mut() {
            d.update(dt, dpos, screen_w, screen_h, &cam);
         }
      }
   }
}

#[instrument]
pub fn render() {
   if let Some(cur_system) = crate::system::cur() {
      let ctx = renderer::Context::get();
      let cam = camera::CAMERA.read().unwrap();
      for ast in cur_system.asteroids() {
         // Test to see if field is in range, or skip if not
         let centre = match ctx.game_to_screen_coords_inrange(
            Vector2::new(ast.pos.x, ast.pos.y).cast::<f32>(),
            ast.radius as f32,
         ) {
            Some(c) => c,
            None => {
               continue;
            }
         };

         // Render all asteroids
         let inner = get_inner(ast);
         for a in inner.asteroids.values() {
            a.render(ctx);
         }
      }

      // Render the debris
      /*
      for d in DEBRIS.lock().unwrap().iter().filter(|d| d.height < 1.0) {
         d.render(ctx);
      }
      */
   }
}

#[instrument]
pub fn render_overlay() {
   if let Some(cur_system) = crate::system::cur() {
      /*
      let ctx = renderer::Context::get();
      for d in DEBRIS.lock().unwrap().iter().filter(|d| d.height >= 1.0) {
         d.render(ctx);
      }
      */
   }
}

#[derive(Debug, PartialEq, Copy, Clone)]
struct LuaAsteroid {
   parent: usize,
   id: AsteroidRef,
}

impl LuaAsteroid {
   pub fn null() -> Self {
      Self {
         parent: 0,
         id: AsteroidRef::null(),
      }
   }

   pub fn with<F, R>(&self, f: F) -> Result<R>
   where
      F: Fn(&Asteroid) -> R,
   {
      if let Some(cur_system) = crate::system::cur()
         && let Some(field) = cur_system.asteroids().get(self.parent)
      {
         let inner = get_inner(field);
         if let Some(ast) = inner.asteroids.get(self.id) {
            Ok(f(ast))
         } else {
            anyhow::bail!("asteroid not found")
         }
      } else {
         anyhow::bail!("asteroid not found")
      }
   }

   pub fn with_mut<F, R>(&self, f: F) -> Result<R>
   where
      F: Fn(&mut Asteroid) -> R,
   {
      if let Some(cur_system) = crate::system::cur()
         && let Some(field) = cur_system.asteroids().get(self.parent)
      {
         let inner = get_inner_mut(field);
         if let Some(ast) = inner.asteroids.get_mut(self.id) {
            Ok(f(ast))
         } else {
            anyhow::bail!("asteroid not found")
         }
      } else {
         anyhow::bail!("asteroid not found")
      }
   }

   pub fn from_c(ast: naevc::LuaAsteroid_t) -> Self {
      Self {
         parent: ast.parent as usize,
         id: AsteroidRef::from_ffi(ast.id),
      }
   }

   pub fn into_c(self) -> naevc::LuaAsteroid_t {
      naevc::LuaAsteroid_t {
         parent: self.parent as i32,
         id: self.id.as_ffi(),
      }
   }
}

impl FromLua for LuaAsteroid {
   fn from_lua(value: mlua::Value, _: &mlua::Lua) -> mlua::Result<Self> {
      match value {
         mlua::Value::UserData(ud) => Ok(*ud.borrow::<Self>()?),
         val => Err(mlua::Error::RuntimeError(format!(
            "unable to convert {} to LuaAsteroid",
            val.type_name()
         ))),
      }
   }
}

/*@
 * @brief Lua bindings to interact with asteroid.
 *
 * @luamod asteroid
 */
impl UserData for LuaAsteroid {
   fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
      /*@
       * @brief Compares two asteroids to see if they are the same.
       *
       *    @luatparam Asteroid a1 Asteroid 1 to compare.
       *    @luatparam Asteroid a2 Asteroid 2 to compare.
       *    @luatreturn boolean true if both asteroids are the same.
       * @luafunc __eq
       */
      methods.add_meta_function(
         MetaMethod::Eq,
         |_, (this, other): (Self, Self)| -> mlua::Result<bool> { Ok(this == other) },
      );
      /*@
       * @brief Gets the asteroid's current (translated) name or notes it is
       * non-existent.
       *
       * @usage tostring(a)
       *
       *    @luatparam Asteroid a Asteroid to convert to string.
       *    @luatreturn string A string representation of the asteroid.
       * if not existent.
       * @luafunc __tostring
       */
      methods.add_meta_function(
         MetaMethod::ToString,
         |_, this: Self| -> mlua::Result<String> {
            Ok(format!("Asteroid( {}, {:?} )", this.parent, this.id))
         },
      );
      /*@
       * @brief Gets all the asteroids in the system.
       *
       *    @luatparam number[opt] field Field to get all asteroids of, or all asteroids if nil.
       *    @luatreturn table t A list of all asteroids in the system.
       * @luafunc getAll
       */
      methods.add_function(
         "getAll",
         |_, field: Option<usize>| -> mlua::Result<Vec<Self>> {
            if let Some(field) = field {
               if let Some(cur_system) = crate::system::cur() {
                  let fields = cur_system.asteroids();
                  if let Some(field) = fields.get(field - 1) {
                     let inner = get_inner(field);
                     Ok(inner
                        .asteroids
                        .iter()
                        .map(|(_, a)| a.as_lua())
                        .collect::<Vec<_>>())
                  } else {
                     Err(mlua::Error::RuntimeError(format!(
                        "Asteroid field '{}' not found in system!",
                        field
                     )))
                  }
               } else {
                  Ok(Vec::new())
               }
            } else {
               let mut asteroids = Vec::new();
               if let Some(cur_system) = crate::system::cur() {
                  for (parent, ast) in cur_system.asteroids().iter().enumerate() {
                     let inner = get_inner(ast);
                     for (id, asteroid) in inner.asteroids.iter() {
                        asteroids.push(asteroid.as_lua())
                     }
                  }
               }
               Ok(asteroids)
            }
         },
      );
      /*@
       * @brief Gets an asteroid in the system.
       *
       *    @luatparam nil|number|Vector|Pilot If not supplied, gets a random
       * asteroid, if not it tries to get the asteroid closest to the Vector or Pilot.
       * In the case of a number, it tries to get a random asteroid from the asteroid
       * field with the number as an id.
       *    @luatreturn Asteroid The closest asteroid or nil if not found.
       * @luafunc get
       */
      methods.add_function(
         "get",
         |lua,
          param: Option<Either<usize, Either<Vec2, mlua::Value>>>|
          -> mlua::Result<Option<Self>> {
            fn get_from_field(field: usize) -> Option<LuaAsteroid> {
               if let Some(cur_system) = crate::system::cur() {
                  let fields = cur_system.asteroids();
                  let parent = range(0..fields.len());
                  let field = &fields[parent];
                  let inner = get_inner(field);
                  let ast: Vec<_> = inner
                     .asteroids
                     .iter()
                     .filter_map(|(k, a)| (a.state == State::Fg).then_some(k))
                     .collect();
                  let n = ast.len();
                  if n == 0 {
                     None
                  } else {
                     let id = ast[range(0..n)];
                     Some(LuaAsteroid { parent, id })
                  }
               } else {
                  None
               }
            }
            fn get_from_pos(pos: Vector2<f64>) -> Option<LuaAsteroid> {
               if let Some(cur_system) = crate::system::cur() {
                  let fields = cur_system.asteroids();
                  let parent = range(0..fields.len());
                  let field = &fields[parent];
                  let inner = get_inner(field);
                  let id = inner
                     .asteroids
                     .iter()
                     .filter(|&(k, a)| (a.state == State::Fg))
                     .map(|(k, a)| (k, (a.pos() - pos).norm_squared()))
                     .min_by(|(_, a), (_, b)| {
                        a.partial_cmp(b).unwrap_or(std::cmp::Ordering::Equal)
                     });
                  id.map(|id| LuaAsteroid { parent, id: id.0 })
               } else {
                  None
               }
            }
            if let Some(param) = param {
               match param {
                  Either::Left(parent) => Ok(get_from_field(parent)),
                  Either::Right(param) => match param {
                     Either::Left(v) => Ok(get_from_pos(v.into())),
                     Either::Right(ud) => match crate::pilot::from_lua(lua, &ud) {
                        Ok(plt) => Ok(get_from_pos(plt.pos())),
                        Err(_) => Ok(None),
                     },
                  },
               }
            } else {
               if let Some(cur_system) = crate::system::cur_mut() {
                  let parent = range(0..cur_system.asteroids().len());
                  Ok(get_from_field(parent))
               } else {
                  Ok(None)
               }
            }
         },
      );
      /*@
       * @brief Checks to see if an asteroid exists.
       *
       *    @luatparam Asteroid a Asteroid to check to see if exists.
       *    @luatreturn boolean true if the asteroid exists, false otherwise.
       * @luafunc exists
       */
      methods.add_method("exists", |_, this, ()| -> mlua::Result<bool> {
         Ok(this.with(|a| a.state == State::Fg).unwrap_or(false))
      });
      /*@
       * @brief Gets the state of an asteroid.
       *
       *    @luatparam Asteroid a Asteroid to check state of.
       *    @luatreturn string State of the asteroid. Can be one of `FG`, `XB`, `BX`,
       * `XX_TO_BG`, `FG_TO_BG`, `BG_TO_FG`, `BG_TO_XX`, or `XX`.
       * @luafunc state
       */
      methods.add_method("state", |_, this, ()| -> mlua::Result<&str> {
         Ok(this
            .with(|a| match a.state {
               State::Fg => "FG",
               State::Xb => "XB",
               State::Bx => "BX",
               State::XxToBg => "XX_TO_BG",
               State::FgToBg => "FG_TO_BG",
               State::BgToFg => "BG_TO_FG",
               State::BgToXx => "BG_TO_XX",
               State::Xx => "XX",
            })
            .unwrap_or("XX"))
      });

      /*@
       * @brief Sets the state of an asteroid.
       *
       *    @luatparam Asteroid a Asteroid to set state of.
       *    @luatparam string s State to set. Has to be one of `FG`, `XB`, `BX`,
       * `XX_TO_BG`, `FG_TO_BG`, `BG_TO_FG`, `BG_TO_XX`, or `XX`.
       * @luafunc setState
       */
      methods.add_method_mut(
         "setState",
         |_, this, state: BorrowedStr| -> mlua::Result<()> {
            this.with_mut(|a| {
               Ok(a.state = match &*state {
                  "FG" => State::Fg,
                  "XB" => State::Xb,
                  "BX" => State::Bx,
                  "XX_TO_BG" => State::XxToBg,
                  "FG_TO_BG" => State::FgToBg,
                  "BG_TO_FG" => State::BgToFg,
                  "BG_TO_XX" => State::BgToXx,
                  "XX" => State::Xx,
                  s => {
                     return Err(mlua::Error::RuntimeError(format!(
                        "unknown asteroid state {}",
                        s
                     )));
                  }
               })
            })?;
            Ok(())
         },
      );

      /*@
       * @brief Gets the field the asteroid belongs to (useful for getting more
       * asteroids from the same field).
       *
       *    @luatparam Asteroid a Asteroid to get what field it belongs to.
       *    @luatreturn number ID of the field the asteroid belongs to.
       * @luafunc field
       */
      methods.add_method("field", |_, this, ()| -> mlua::Result<usize> {
         Ok(this.with(|a| a.parent + 1)?)
      });

      /*@
       * @brief Gets the position of an asteroid.
       *
       *    @luatparam Asteroid a Asteroid to get position of.
       *    @luatreturn Vector Position of the asteroid.
       * @luafunc pos
       */
      methods.add_method("pos", |_, this, ()| -> mlua::Result<Vec2> {
         Ok(this.with(|a| Vec2::new(a.solid.pos.x, a.solid.pos.y))?)
      });

      /*@
       * @brief Gets the velocity of an asteroid.
       *
       *    @luatparam Asteroid a Asteroid to get velocity of.
       *    @luatreturn Vector Position of the asteroid.
       * @luafunc vel
       */
      methods.add_method("vel", |_, this, ()| -> mlua::Result<Vec2> {
         Ok(this.with(|a| Vec2::new(a.solid.vel.x, a.solid.vel.y))?)
      });

      /*@
       * @brief Sets the position of an asteroid.
       *
       *    @luatparam Asteroid a Asteroid to set position of.
       *    @luatparam vec2 v Position to set to.
       * @luafunc setPos
       */
      methods.add_method_mut("setPos", |_, this, pos: Vec2| -> mlua::Result<()> {
         let pos = pos.into_vector2();
         Ok(this.with_mut(|a| {
            a.solid.pos.x = pos.x;
            a.solid.pos.y = pos.y;
         })?)
      });

      /*@
       * @brief Sets the velocity of an asteroid.
       *
       *    @luatparam Asteroid a Asteroid to set velocity of.
       *    @luatparam vec2 v Velocity to set to.
       * @luafunc setVel
       */
      methods.add_method_mut("setVel", |_, this, vel: Vec2| -> mlua::Result<()> {
         let vel = vel.into_vector2();
         Ok(this.with_mut(|a| {
            a.solid.vel.x = vel.x;
            a.solid.vel.y = vel.y;
         })?)
      });

      /*@
       * @brief Gets whether or not an asteroid got scanned.
       *
       *    @luatparam Asteroid a Asteroid to get scanned state of.
       *    @luatreturn boolean Whether or not the asteroid was scanned.
       * @luafunc scanned
       */
      methods.add_method("scanned", |_, this, ()| -> mlua::Result<bool> {
         Ok(this.with(|a| a.scanned)?)
      });

      /*@
       * @brief Gets the time left on the asteroid.
       *
       *    @luatparam Asteroid a Asteroid to get time left on.
       *    @luatreturn number Seconds left before the asteroid is unavailable.
       *    @luatreturn number Total number of seconds the asteroid had to live.
       * @luafunc timer
       */
      methods.add_method("timer", |_, this, ()| -> mlua::Result<(f64, f64)> {
         Ok(this.with(|a| (a.timer, a.timer_max))?)
      });

      /*@
       * @brief Sets the time left on the asteroid.
       *
       *    @luatparam Asteroid a Asteroid to set time left of.
       *    @luatparam number Time left to set to in seconds.
       * @luafunc setTimer
       */
      methods.add_method_mut("setTimer", |_, this, t: f64| -> mlua::Result<()> {
         Ok(this.with_mut(|a| {
            a.timer = t;
            a.timer_max = a.timer_max.max(a.timer);
         })?)
      });

      /*@
       * @brief Gets the armour (health) left on the asteroid.
       *
       *    @luatparam Asteroid a Asteroid to get armour of.
       *    @luatreturn number Armour left before the asteroid is destroyed.
       * @luafunc armour
       */
      methods.add_method("armour", |_, this, ()| -> mlua::Result<f64> {
         Ok(this.with(|a| a.armour)?)
      });

      /*@
       * @brief Sets the armour of the asteroid.
       *
       *    @luatparam Asteroid a Asteroid to set armour of.
       *    @luatparam number Amount to set the armour to (negative values will
       * explode it).
       * @luafunc setArmour
       */
      methods.add_method("setArmour", |_, this, armour: f64| -> mlua::Result<()> {
         Ok(this.with_mut(|a| {
            a.armour = armour;
            if a.armour <= 0. {
               a.explode(-1, 0.);
            }
         })?)
      });

      /*@
       * @brief Gets the alert range of an asteroid.
       *
       *    @luatparam Asteroid a Asteroid to get alert range of.
       *    @luatreturn number Alert range of the asteroid.
       * @luafunc alertRange
       */
      methods.add_method("alertRange", |_, this, ()| -> mlua::Result<f64> {
         Ok(this.with(|a| a.atype.alert_range)?)
      });

      /*@
       * @brief Gets the materials the asteroid can potentially drop.
       *
       *    @luatparam Asteroid a Asteroid to get materials of.
       *    @luatreturn table Table containing the materials.
       * @luafunc materials
       */
      methods.add_method("materials", |lua, this, ()| -> mlua::Result<mlua::Table> {
         this.with(|a| {
            let t = match lua.create_table() {
               Ok(t) => t,
               Err(e) => {
                  return Err(e);
               }
            };
            for m in &a.atype.material {
               if let Ok(tm) = lua.create_table() {
                  tm.set("commodity", m.material);
                  tm.set("quantity", m.quantity);
                  tm.set("rarity", m.rarity);
                  t.push(tm);
               }
            }
            Ok(t)
         })?
      });
   }
}

use mlua::{Value, ffi};
pub fn open_asteroid(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData> {
   let proxy = lua.create_proxy::<LuaAsteroid>()?;

   // Only add stuff as necessary
   if let mlua::Value::Nil = lua.named_registry_value("push_asteroid")? {
      let push_asteroid = lua.create_function(|lua, (parent, id): (i64, i64)| {
         let ast = LuaAsteroid {
            parent: parent as usize,
            id: AsteroidRef::from_ffi(id),
         };
         lua.create_userdata(ast)
      })?;
      lua.set_named_registry_value("push_asteroid", push_asteroid)?;

      let get_asteroid = lua.create_function(|_, mut ud: mlua::UserDataRefMut<LuaAsteroid>| {
         let ast: *mut LuaAsteroid = &mut *ud;
         Ok(Value::LightUserData(mlua::LightUserData(
            ast as *mut c_void,
         )))
      })?;
      lua.set_named_registry_value("get_asteroid", get_asteroid)?;
   }

   Ok(proxy)
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C-unwind" fn luaL_checkasteroid(
   L: *mut mlua::lua_State,
   idx: c_int,
) -> naevc::LuaAsteroid_t {
   unsafe {
      let ast = lua_toasteroid(L, idx);
      if LuaAsteroid::from_c(ast) == LuaAsteroid::null() {
         ffi::luaL_typerror(L, idx, c"asteroid".as_ptr() as *const c_char);
      }
      ast
   }
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C-unwind" fn luaL_validasteroid(
   L: *mut mlua::lua_State,
   idx: c_int,
) -> *const Asteroid {
   let ast = unsafe {
      let ast = lua_toasteroid(L, idx);
      if LuaAsteroid::from_c(ast) == LuaAsteroid::null() {
         ffi::luaL_typerror(L, idx, c"asteroid".as_ptr() as *const c_char);
      }
      ast
   };
   if let Some(cur_system) = crate::system::cur_mut() {
      let field = &cur_system.asteroids()[ast.parent as usize];
      let inner = get_inner(field);
      match inner.asteroids.get(AsteroidRef::from_ffi(ast.id)) {
         Some(ast) => ast,
         None => std::ptr::null(),
      }
   } else {
      std::ptr::null()
   }
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C-unwind" fn lua_isasteroid(L: *mut mlua::lua_State, idx: c_int) -> c_int {
   (LuaAsteroid::from_c(lua_toasteroid(L, idx)) != LuaAsteroid::null()) as c_int
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C-unwind" fn lua_pushasteroid(L: *mut mlua::lua_State, ast: naevc::LuaAsteroid_t) {
   unsafe {
      ffi::lua_getfield(L, ffi::LUA_REGISTRYINDEX, c"push_asteroid".as_ptr());
      ffi::lua_pushinteger(L, ast.parent as i64);
      ffi::lua_pushinteger(L, ast.id);
      ffi::lua_call(L, 2, 1);
   }
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C-unwind" fn lua_toasteroid(
   L: *mut mlua::lua_State,
   idx: c_int,
) -> naevc::LuaAsteroid_t {
   unsafe {
      let idx = ffi::lua_absindex(L, idx);
      ffi::lua_getfield(L, ffi::LUA_REGISTRYINDEX, c"get_asteroid".as_ptr());
      ffi::lua_pushvalue(L, idx);
      let ast = match ffi::lua_pcall(L, 1, 1, 0) {
         ffi::LUA_OK => {
            let ptr = ffi::lua_touserdata(L, -1) as *mut LuaAsteroid;
            if ptr.is_null() {
               LuaAsteroid::null().into_c()
            } else {
               (*ptr).into_c()
            }
         }
         _ => LuaAsteroid::null().into_c(),
      };
      ffi::lua_pop(L, 1);
      ast
   }
}

#[derive(Default)]
struct AnchorInner {
   asteroids: SlotMap<AsteroidRef, Asteroid>,
   has_exclusion: bool,
   bvh: Bvh<AsteroidRef, Aabb<2>>,
}

impl AnchorInner {
   fn new() -> Self {
      Default::default()
   }

   fn update_bvh(&mut self) {
      self.bvh.clear();
      for (k, a) in &self.asteroids {
         // Only foreground ones are allowed
         if a.state == State::Fg {
            self.bvh.insert(k, a.aabb());
         }
      }
   }
}

#[instrument]
#[unsafe(no_mangle)]
pub extern "C" fn asteroids_init() {
   if let Some(cur_system) = crate::system::cur_mut() {
      let mut density_max = 0.0;
      for ast in cur_system.asteroids_mut() {
         if ast.inner.is_null() {
            ast.inner = Box::into_raw(Box::new(AnchorInner {
               asteroids: SlotMap::with_key(),
               has_exclusion: false,
               bvh: Bvh::default(),
            })) as *mut naevc::AsteroidInner;
         }
         let mut inner = get_inner_mut(ast);

         // TODO maybe enforce uniqueness?
         let groups = unsafe { array::array_as_slice(ast.groups) };
         {
            let mut debris_gfx = DEBRIS_GFX.write().unwrap();
            debris_gfx.clear();
            for g in groups {
               let g = unsafe { &*(*g as *mut TypeGroup) };
               for (t, _) in g.types.iter() {
                  for gfx in &t.gfx {
                     debris_gfx.push(gfx.clone());
                  }
               }
            }
         }

         // Add asteroids to the anchor
         inner.asteroids.clear();
         for i in 0..ast.nmax {
            if let Some(mut a) = Asteroid::try_new(ast, true) {
               let r = rng::<f32>();
               if r > 0.6 {
                  a.state = State::Fg;
               } else if r > 0.8 {
                  a.state = State::Xb;
               } else if r > 0.9 {
                  a.state = State::Bx;
               } else {
                  a.state = State::Xx;
               }
               a.timer_max = 30.0 * rng::<f64>();
               a.timer = a.timer_max;
               inner.asteroids.insert_with_key(|k| {
                  a.id = k;
                  a
               });
            }
         }
         inner.update_bvh();

         density_max = ast.density.max(density_max);
      }

      // TODO set debris based density_max
      let (screen_w, screen_h) = {
         let dims = renderer::Context::get().dimensions.read().unwrap();
         (dims.view_width, dims.view_height)
      };
      let ndebris = density_max as f32
         * 100.
         * (screen_w + 2. * DEBRIS_BUFFER * screen_h + 2. * DEBRIS_BUFFER)
         / (renderer::MIN_WIDTH_F32 * renderer::MIN_HEIGHT_F32);
      let mut debris = DEBRIS.lock().unwrap();
      for _ in 0..ndebris.max(0.0).round() as usize {
         debris.push(Debris::new());
      }
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn asteroid_initAnchor(anc: *mut naevc::AsteroidAnchor) {
   let mut anc = unsafe { &mut *anc };
   anc.density = naevc::ASTEROID_DEFAULT_DENSITY;
   anc.groups = Array::new(Vec::new()).into_ptr();
   anc.groupsw = Array::new(Vec::new()).into_ptr();
   anc.radius = naevc::ASTEROID_DEFAULT_RADIUS;
   anc.maxspeed = naevc::ASTEROID_DEFAULT_MAXSPEED;
   anc.maxspin = naevc::ASTEROID_DEFAULT_MAXSPIN;
   anc.accel = naevc::ASTEROID_DEFAULT_ACCEL;
   anc.inner = unsafe { Box::into_raw(Box::new(AnchorInner::new())) as *mut naevc::AsteroidInner };
}

#[unsafe(no_mangle)]
pub extern "C" fn asteroid_freeAnchor(anc: *mut naevc::AsteroidAnchor) {
   let mut anc = unsafe { &mut *anc };
   unsafe {
      let _ = Array::from_ptr(anc.groups);
      let _ = Array::from_ptr(anc.groupsw);
   }
   if !anc.inner.is_null() {
      unsafe {
         let _ = Box::from_raw(anc.inner);
      }
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn asteroids_update(dt: f64) {
   update(dt)
}

#[unsafe(no_mangle)]
pub extern "C" fn asteroids_render() {
   render();
}

#[unsafe(no_mangle)]
pub extern "C" fn asteroids_renderOverlay() {
   render_overlay();
}

#[unsafe(no_mangle)]
pub extern "C" fn astgroup_getAll() -> *const *const TypeGroup {
   let vec: Vec<*const TypeGroup> = GROUPS.iter().map(|(k, v)| v as *const TypeGroup).collect();
   array::Array::new(vec).into_ptr()
}

#[unsafe(no_mangle)]
pub extern "C" fn astgroup_getName(name: *const c_char) -> *const TypeGroup {
   let name = unsafe { CStr::from_ptr(name) };
   match GROUPS.get(&*name.to_string_lossy()) {
      Some(at) => at,
      None => std::ptr::null(),
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn astgroup_name(at: *const TypeGroup) -> *const c_char {
   let at = unsafe { &*at };
   at.cname.as_ptr()
}

#[unsafe(no_mangle)]
pub extern "C" fn ast_get(ast: *const naevc::AsteroidAnchor, id: i64) -> *const Asteroid {
   let ast = unsafe { &*ast };
   let inner = get_inner(ast);
   match inner.asteroids.get(AsteroidRef::from_ffi(id)) {
      Some(ast) => ast,
      None => std::ptr::null(),
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn ast_id(ast: *const Asteroid) -> i64 {
   let ast = unsafe { &*ast };
   ast.id.as_ffi()
}

#[unsafe(no_mangle)]
pub extern "C" fn ast_parent(ast: *const Asteroid) -> c_int {
   let ast = unsafe { &*ast };
   ast.parent as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn ast_state(ast: *const Asteroid) -> naevc::AsteroidState {
   let ast = unsafe { &*ast };
   ast.state.to_ffi()
}

#[unsafe(no_mangle)]
pub extern "C" fn ast_solid(ast: *const Asteroid) -> *const naevc::Solid {
   let ast = unsafe { &*ast };
   &ast.solid
}

#[unsafe(no_mangle)]
pub extern "C" fn ast_gfx_width(ast: *const Asteroid) -> f64 {
   let ast = unsafe { &*ast };
   (match &*ast.gfx {
      GfxType::Single(gfx) => gfx.texture.sw,
      GfxType::Sprite(gfx) => gfx.texture.sw,
   }) as f64
}

#[unsafe(no_mangle)]
pub extern "C" fn ast_test_collide(
   ast: *const Asteroid,
   at: *const Polygon,
   ap: *const Vector2<f64>,
   crash: *mut Vector2<f64>,
) -> i32 {
   let ast = unsafe { &*ast };
   let at = unsafe { &*at };
   let ap = unsafe { *ap };
   let crash: &mut [Vector2<f64>] = unsafe { std::slice::from_raw_parts_mut(crash, 2) };
   let bp = ast.pos();
   let hit = match &*ast.gfx {
      GfxType::Single(gfx) => {
         let bt = gfx.poly.view(ast.angle);
         let t = nalgebra::Isometry2::new(ap - bp, ast.angle);
         bt.intersect_polygon_transform(at, &t)
      }
      GfxType::Sprite(gfx) => {
         let bt = gfx.poly.view(ast.angle);
         bt.intersect_polygon(at, ap - bp)
      }
   };
   for (k, h) in hit.iter().enumerate() {
      crash[k] = *h + bp;
   }
   hit.len() as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn ast_scanned(ast: *const Asteroid) -> c_int {
   let ast = unsafe { &*ast };
   ast.scanned as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn ast_set_scanned(ast: *mut Asteroid, set: c_int) {
   let ast = unsafe { &mut *ast };
   ast.scanned = set != 0;
}

/// Returns a copy
#[unsafe(no_mangle)]
pub extern "C" fn ast_poly(ast: *const Asteroid) -> *mut Polygon {
   let ast = unsafe { &*ast };
   match &*ast.gfx {
      GfxType::Single(gfx) => collide::polygon::poly_rotate(&gfx.poly, ast.angle),
      GfxType::Sprite(gfx) => Box::into_raw(Box::new(gfx.poly.view(ast.angle).clone())),
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn asteroid_closestPilot(
   ast: *const naevc::AsteroidAnchor,
   x: f64,
   y: f64,
   d: *mut f64,
) -> i64 {
   let ast = unsafe { &*ast };
   let inner = get_inner(ast);
   let pos = Vector2::new(x, y);
   match inner
      .asteroids
      .iter()
      .filter_map(|(k, a)| {
         // TODO
         //if ( !pilot_inRangeAsteroid( player.p, k, as->parent ) )
         //continue;
         if a.state == State::Fg {
            let ap = a.pos();
            Some((k, (ap - pos).norm_squared()))
         } else {
            None
         }
      })
      .min_by(|(_, a), (_, b)| a.partial_cmp(b).unwrap_or(std::cmp::Ordering::Equal))
   {
      Some((k, a)) => k.as_ffi(),
      None => AsteroidRef::null().as_ffi(),
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn asteroid_inField(p: *const Vector2<f64>) -> c_int {
   let p = unsafe { *p };
   match infield(p) {
      Some(v) => v as c_int,
      None => -1,
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn asteroids_hasCommodity(field: *const naevc::AsteroidAnchor, com: i64) -> c_int {
   let com = CommodityRef::from_ffi(com);
   let field = unsafe { &*field };
   let groups = unsafe { array::array_as_slice(field.groups) };
   for g in groups {
      let g = unsafe { &*(*g as *mut TypeGroup) };
      for (t, _) in g.types.iter() {
         for m in t.material.iter() {
            if m.material == com {
               return true as c_int;
            }
         }
      }
   }
   false as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn asteroids_computeInternals(a: *mut naevc::AsteroidAnchor) {
   let a = unsafe { &mut *a };
   a.area = std::f64::consts::PI * a.radius * a.radius;
   a.nmax = (a.area / naevc::ASTEROID_REF_AREA * a.density).round() as i32;
   a.margin = a.maxspeed * a.maxspeed / (4.0 * a.accel) + 50.0;
   a.groupswtotal = 0.0;
   let groupsw = unsafe { array::array_as_slice(a.groupsw) };
   for w in groupsw {
      a.groupswtotal += w;
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn asteroid_hit(
   a: *mut Asteroid,
   dmg: *const naevc::Damage,
   max_rarity: i32,
   mine_bonus: f64,
) {
   let a = unsafe { &mut *a };
   let dmg = unsafe { &*dmg };
   let absorb = 0.99f64.powf(a.atype.absorb - dmg.penetration);
   let mut darmour = 0.0f64;
   crate::damagetype::dtype_calcDamage(
      std::ptr::null_mut(),
      &mut darmour,
      absorb,
      dmg,
      std::ptr::null(),
   );
   a.armour -= darmour;
   if a.armour <= 0.0 {
      a.explode(max_rarity, mine_bonus);
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn asteroid_explode(a: *mut Asteroid, max_rarity: i32, mine_bonus: f64) {
   let a = unsafe { &mut *a };
   a.explode(max_rarity, mine_bonus);
}

#[unsafe(no_mangle)]
pub extern "C" fn asteroid_collideQueryIL(
   anc: *mut naevc::AsteroidAnchor,
   x1: c_int,
   y1: c_int,
   x2: c_int,
   y2: c_int,
) -> *const naevc::AsteroidRef {
   let anc = unsafe { &mut *anc };
   let inner = get_inner(anc);
   let query = Aabb::from_min_max(
      Vector2::new(x1 as f32, y1 as f32),
      Vector2::new(x2 as f32, y2 as f32),
   );
   // TODO probably something better here, but we need to move to Rust and just pass iterators
   // around..
   static HITS: LazyLock<Mutex<Array<naevc::AsteroidRef>>> = LazyLock::new(Default::default);
   let mut hits = HITS.lock().unwrap();
   hits.clear();
   for a in inner.bvh.overlaps(&query) {
      // No need for additional checks as only stuff that collides is in the bvh
      hits.push(a.as_ffi());
   }
   hits.as_ptr()
}

#[unsafe(no_mangle)]
pub extern "C" fn asteroids_inField(p: *const naevc::vec2) -> c_int {
   if let Some(cur_system) = crate::system::cur() {
      let p = unsafe { &*p };
      let p = Vector2::new(p.x, p.y);

      // Always return -1 if in an exclusion zone.
      for exc in cur_system.astexclude() {
         let ep = Vector2::new(exc.pos.x, exc.pos.y);
         if (p - ep).norm_squared() <= exc.radius * exc.radius {
            return -1;
         }
      }

      // Check if in asteroid field.
      for (i, ast) in cur_system.asteroids().into_iter().enumerate() {
         let ap = Vector2::new(ast.pos.x, ast.pos.y);
         if (p - ap).norm_squared() <= ast.radius * ast.radius {
            return i as c_int;
         }
      }
   }
   -1
}

#[unsafe(no_mangle)]
pub extern "C" fn asttype_getName(name: *const c_char) -> *const naevc::AsteroidType {
   let s = unsafe { CStr::from_ptr(name).to_str().unwrap() };
   match Type::get_r(s) {
      Ok(t) => Arc::as_ptr(&t) as *const naevc::AsteroidType,
      Err(e) => {
         warn_err!(e);
         std::ptr::null()
      }
   }
}
