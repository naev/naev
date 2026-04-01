#![allow(dead_code)]
use crate::commodity::CommodityRef;
use crate::hook::HookParam;
use crate::pilot;
use crate::player;
use formatx::formatx;
use gettext::{gettext, ngettext};
use helpers::{ReferenceC, atomicfloat::AtomicF64};
use nalgebra::Vector2;
use nlog::{warn, warn_err};
use slotmap::{Key, KeyData, SlotMap};
use std::sync::atomic::Ordering;
use std::sync::{LazyLock, Mutex};

const GATHER_DIST: f64 = 30.0;
const GATHER_DIST2: f64 = GATHER_DIST * GATHER_DIST;
const NOSCOOP_DELAY: f64 = 2.0;

static NOSCOOP_TIMER: AtomicF64 = AtomicF64::new(0.0);

/// Represents somethnig floating in space that can be gathered
pub struct Gatherable {
   /// Commodity that is picked up
   commodity: CommodityRef,
   /// Position
   pos: Vector2<f64>,
   /// Velocity
   vel: Vector2<f64>,
   /// Time to live, will disappear when it's over
   ttl: f64,
   /// Amount that gets picked up
   quantity: i32,
   /// Whether or not only the player can gather it
   player_only: bool,
   // TODO change graphic handling
   sx: i32,
   sy: i32,
}
impl Gatherable {
   fn new(
      commodity: CommodityRef,
      pos: Vector2<f64>,
      vel: Vector2<f64>,
      ttl: f64,
      quantity: i32,
      player_only: bool,
   ) -> Self {
      Self {
         commodity,
         pos,
         vel,
         ttl,
         quantity,
         player_only,
         sx: 0,
         sy: 0,
      }
   }

   fn gather_pilot(&self, p: &mut pilot::PilotWrapper) -> bool {
      let f = |f| p.0.flags[f as usize] != 0;
      // Must not be dead
      if f(naevc::PILOT_DEAD) || f(naevc::PILOT_DELETE) {
         return false;
      }
      // Must not be hidden or invisible
      if f(naevc::PILOT_HIDE) || f(naevc::PILOT_INVISIBLE) {
         return false;
      }
      // Disabled pilots can pick up stuff
      if f(naevc::PILOT_DISABLED) {
         return false;
      }

      let isplayer = f(naevc::PILOT_PLAYER);

      let q = unsafe { naevc::pilot_cargoAdd(p.0, self.commodity.as_ffi(), self.quantity, 0) };
      if q > 0 {
         let msg = self.commodity.call(|c| {
            formatx!(
               ngettext(
                  "{} tonne of {} gathered.",
                  "{}
                  tonnes of {} gathered.",
                  q as u64
               ),
               q,
               c.name()
            )
            .unwrap_or_else(|e| {
               warn_err!(e);
               "broken  message".to_string()
            })
         });
         match msg {
            Ok(msg) => player::message(&msg),
            Err(e) => warn_err!(e),
         };

         let hparam = [
            HookParam::Commodity(self.commodity),
            HookParam::Number(q as f64),
         ];
         crate::hook::run_param_deferred("gather", &hparam);

         if unsafe { naevc::pilot_cargoFree(p.0) < 1 } && isplayer {
            player::message(gettext("Your ship's cargo is full."));
         }
         return true;
      } else if isplayer && NOSCOOP_TIMER.load(Ordering::Relaxed) > NOSCOOP_DELAY {
         NOSCOOP_TIMER.store(0.0, Ordering::Relaxed);
         player::message(gettext(
            "Cannot gather material: no more cargo space available.",
         ));
      }

      false
   }
}

slotmap::new_key_type! {
pub struct GatherableRef;
}
impl GatherableRef {
   pub fn from_ffi(value: i64) -> Self {
      Self(KeyData::from_ffi(value as u64))
   }
}

struct Manager {
   data: SlotMap<GatherableRef, Gatherable>,
   query: naevc::IntList,
}
unsafe impl Send for Manager {}
impl Manager {
   fn new() -> Self {
      use std::mem::MaybeUninit;
      let mut query = MaybeUninit::<naevc::IntList>::uninit();
      unsafe { naevc::il_create(query.as_mut_ptr(), 1) };
      let query = unsafe { query.assume_init() };
      Self {
         data: SlotMap::with_key(),
         query,
      }
   }

   fn split_borrow_mut(
      &mut self,
   ) -> (&mut SlotMap<GatherableRef, Gatherable>, &mut naevc::IntList) {
      (&mut self.data, &mut self.query)
   }
}

static GATHERABLES: LazyLock<Mutex<Manager>> = LazyLock::new(|| Mutex::new(Manager::new()));

pub fn render() {
   let manager = GATHERABLES.lock().unwrap();
   let commodities = crate::commodity::COMMODITIES.read().unwrap();
   for (_, g) in manager.data.iter() {
      if let Some(c) = commodities.get(g.commodity) {
         if let Some(gfx) = &c.gfx_space {
            unsafe {
               naevc::gl_renderSprite(
                  gfx as *const renderer::texture::Texture as *const naevc::glTexture,
                  g.pos.x,
                  g.pos.y,
                  g.sx,
                  g.sy,
                  std::ptr::null(),
               );
            }
         } else {
            warn!(
               "commodity '{}' used in gatherable has no gfx_space!",
               c.name
            );
         }
      } else {
         warn!("commodity '{:?}' not found!", g.commodity);
      }
   }
}

pub fn update(dt: f64) {
   // TODO fetch_add support, maybe use library?
   let val = NOSCOOP_TIMER.load(Ordering::SeqCst);
   NOSCOOP_TIMER.store(val + dt, Ordering::SeqCst);

   let pilot_stack = pilot::get_mut();

   let mut manager = GATHERABLES.lock().unwrap();
   let (data, query) = manager.split_borrow_mut();
   data.retain(|_, g| {
      g.ttl -= dt;
      if g.ttl < 0.0 {
         return false;
      }
      g.pos += g.vel * dt;

      if g.player_only {
         if let Some(mut p) = pilot::player()
            && (p.pos() - g.pos).norm_squared() <= GATHER_DIST2
         {
            return g.gather_pilot(&mut p);
         }
         return false;
      }

      let x = g.pos.x.round() as i32;
      let y = g.pos.y.round() as i32;
      let r = GATHER_DIST as i32;
      let s = unsafe {
         naevc::pilot_collideQueryIL(query, x - r, y - r, x + r, y + r);
         naevc::il_size(query)
      };
      for i in 0..s {
         let p = unsafe { &mut pilot_stack[naevc::il_get(query, i, 0) as usize] };
         if p.0.flags[naevc::PILOT_CARRIED as usize] != 0 {
            continue;
         }

         if (p.pos() - g.pos).norm_squared() > GATHER_DIST2 {
            continue;
         }

         if g.gather_pilot(p) {
            return true;
         }
      }

      false
   });
}

use std::ffi::{c_double, c_int};

#[unsafe(no_mangle)]
pub extern "C" fn _gatherable_init(
   com: i64,
   pos: *const Vector2<f64>,
   vel: *const Vector2<f64>,
   lifeleng: c_double,
   qtt: c_int,
   player_only: c_int,
) -> i64 {
   let com = CommodityRef::from_ffi(com);
   let g = Gatherable::new(
      com,
      unsafe { *pos },
      unsafe { *vel },
      lifeleng,
      qtt,
      player_only != 0,
   );
   GATHERABLES.lock().unwrap().data.insert(g).as_ffi()
}

#[unsafe(no_mangle)]
pub extern "C" fn _gatherable_getClosest(pos: *const Vector2<f64>, rad: f64) -> i64 {
   let pos = unsafe { *pos };
   let gclosest = GATHERABLES
      .lock()
      .unwrap()
      .data
      .iter()
      .map(|(k, v)| (k, (pos - v.pos).norm_squared()))
      .min_by(|a, b| a.1.partial_cmp(&b.1).unwrap_or(std::cmp::Ordering::Equal));

   if let Some(g) = gclosest
      && g.1 < rad
   {
      g.0.as_ffi()
   } else {
      GatherableRef::null().as_ffi()
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn _gatherable_getPos(
   pos: *mut Vector2<f64>,
   vel: *mut Vector2<f64>,
   id: i64,
) -> c_int {
   let manager = GATHERABLES.lock().unwrap();
   match manager.data.get(GatherableRef::from_ffi(id)) {
      Some(g) => {
         unsafe {
            *pos = g.pos;
            *vel = g.vel;
         }
         1
      }
      None => {
         unsafe {
            *pos = Vector2::default();
            *vel = Vector2::default();
         }
         0
      }
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn _gatherable_render() {
   render()
}

#[unsafe(no_mangle)]
pub extern "C" fn _gatherable_update(dt: c_double) {
   update(dt)
}

#[unsafe(no_mangle)]
pub extern "C" fn _gatherable_free() {
   GATHERABLES.lock().unwrap().data.clear();
}
