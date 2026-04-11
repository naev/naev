#![allow(dead_code, unused)]
use crate::array;
use crate::commodity::CommodityRef;
use crate::pilot;
use crate::rng::{range, rng};
use anyhow::Context as AnyhowContext;
use anyhow::Result;
use audio::{AudioBuilder, AudioType};
use collide::polygon::Polygon;
use collide::polygon::SpinPolygon;
use helpers::ReferenceC;
use naev_core::{nxml, nxml_err_attr_missing, nxml_warn_node_unknown};
use nalgebra::Vector2;
use nlog::{debugx, warn, warn_err};
use rayon::prelude::*;
use renderer::texture::{Texture, TextureBuilder};
use renderer::{Context, ContextWrapper};
use slotmap::{Key, KeyData, SlotMap};
use std::collections::HashMap;
use std::ffi::{CStr, CString, OsStr, c_char, c_int};
use std::mem::MaybeUninit;
use std::path::{Path, PathBuf};
use std::sync::{Arc, LazyLock};
use tracing::instrument;

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
   penetration: f64,
   exp_radius: f64,
   alert_range: f64,
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

#[derive(Debug, Default)]
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

   fn to_ffi(&self) -> naevc::AsteroidState {
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

#[derive(Debug)]
pub struct Asteroid {
   id: AsteroidRef,
   parent: i32,
   state: State,
   atype: Arc<Type>,
   gfx: Arc<GfxType>,
   armour: f64,

   solid: naevc::Solid,
   ang: f64,
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
            parent: field.id,
            state: State::Xx,
            atype: atype.clone(),
            gfx,
            armour,

            solid,

            ang: std::f64::consts::TAU * rng::<f64>(),
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

   fn pos(&self) -> Vector2<f64> {
      Vector2::new(self.solid.pos.x, self.solid.pos.y)
   }

   fn vel(&self) -> Vector2<f64> {
      Vector2::new(self.solid.vel.x, self.solid.vel.y)
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
      debugx!(
         gettext::ngettext(
            "Loaded {} Asteroid in {:.3} s",
            "Loaded {} Asteroids in {:.3} s",
            n as u64
         ),
         n,
         start.elapsed().as_secs_f32()
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
   if let Some(cur_system) = crate::system::cur() {}
}

#[instrument]
#[unsafe(no_mangle)]
pub extern "C" fn _asteroids_init() {
   if let Some(cur_system) = crate::system::cur_mut() {
      let mut density_max = 0.0;
      for ast in cur_system.asteroids_mut() {
         // TODO add graphics to debris

         // Build quadtree
         if ast.qt_init != 0 {
            unsafe {
               naevc::qt_destroy(&mut ast.qt);
            }
         }
         let qx = ast.pos.x.round() as i32;
         let qy = ast.pos.y.round() as i32;
         let qr = ast.radius.ceil() as i32;
         unsafe {
            naevc::qt_create(&mut ast.qt, qx - qr, qy - qr, qx + qr, qy + qr, 2, 5);
         }
         ast.qt_init = 1;

         // Add asteroids to the anchor
         let asteroids = unsafe { &mut *(ast.asteroids as *mut SlotMap<AsteroidRef, Asteroid>) };
         asteroids.clear();
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
               asteroids.insert_with_key(|k| {
                  a.id = k;
                  a
               });
            }
         }

         density_max = ast.density.max(density_max);
      }
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn _asteroids_update(dt: f64) {
   update(dt)
}

#[unsafe(no_mangle)]
pub extern "C" fn _asteroids_render() {}

#[unsafe(no_mangle)]
pub extern "C" fn _asteroids_renderOverlay() {}

#[unsafe(no_mangle)]
pub extern "C" fn _astgroup_getAll() -> *const *const TypeGroup {
   let vec: Vec<*const TypeGroup> = GROUPS.iter().map(|(k, v)| v as *const TypeGroup).collect();
   array::Array::new(vec).into_ptr()
}

#[unsafe(no_mangle)]
pub extern "C" fn _astgroup_getName(name: *const c_char) -> *const TypeGroup {
   let name = unsafe { CStr::from_ptr(name) };
   match GROUPS.get(&*name.to_string_lossy()) {
      Some(at) => &*at,
      None => std::ptr::null(),
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn _astgroup_name(at: *const TypeGroup) -> *const c_char {
   let at = unsafe { &*at };
   at.cname.as_ptr()
}

#[unsafe(no_mangle)]
pub extern "C" fn _ast_get(ast: *const naevc::AsteroidAnchor, id: i64) -> *const Asteroid {
   let ast = unsafe { &*ast };
   let asteroids = unsafe { &mut *(ast.asteroids as *mut SlotMap<AsteroidRef, Asteroid>) };
   match asteroids.get(AsteroidRef::from_ffi(id)) {
      Some(ast) => ast,
      None => std::ptr::null(),
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn _ast_id(ast: *const Asteroid) -> i64 {
   let ast = unsafe { &*ast };
   ast.id.as_ffi()
}

#[unsafe(no_mangle)]
pub extern "C" fn _ast_parent(ast: *const Asteroid) -> i32 {
   let ast = unsafe { &*ast };
   ast.parent
}

#[unsafe(no_mangle)]
pub extern "C" fn _ast_state(ast: *const Asteroid) -> naevc::AsteroidState {
   let ast = unsafe { &*ast };
   ast.state.to_ffi()
}

#[unsafe(no_mangle)]
pub extern "C" fn _ast_solid(ast: *const Asteroid) -> *const naevc::Solid {
   let ast = unsafe { &*ast };
   &ast.solid
}

#[unsafe(no_mangle)]
pub extern "C" fn _ast_gfx_width(ast: *const Asteroid) -> f64 {
   let ast = unsafe { &*ast };
   match &*ast.gfx {
      GfxType::Single(gfx) => gfx.texture.sw as f64,
      GfxType::Sprite(gfx) => gfx.texture.sw as f64,
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn _ast_test_collide(
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
   let bt = match &*ast.gfx {
      // TODO have to handle polygon rotation
      GfxType::Single(gfx) => gfx.poly.view(ast.ang),
      GfxType::Sprite(gfx) => gfx.poly.view(ast.ang),
   };
   let hit = bt.intersect_polygon(at, ap - bp);
   for (k, h) in hit.iter().enumerate() {
      crash[k] = *h + bp;
   }
   hit.len() as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn _ast_scanned(ast: *const Asteroid) -> c_int {
   let ast = unsafe { &*ast };
   ast.scanned as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn _ast_set_scanned(ast: *mut Asteroid, set: c_int) {
   let ast = unsafe { &mut *ast };
   ast.scanned = set != 0;
}

#[unsafe(no_mangle)]
pub extern "C" fn _ast_poly(ast: *const Asteroid) -> *const Polygon {
   let ast = unsafe { &*ast };
   match &*ast.gfx {
      // TODO have to handle polygon rotation
      GfxType::Single(gfx) => gfx.poly.view(ast.ang),
      GfxType::Sprite(gfx) => gfx.poly.view(ast.ang),
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn _asteroid_closestPilot(
   ast: *const naevc::AsteroidAnchor,
   x: f64,
   y: f64,
   d: *mut f64,
) -> i64 {
   let ast = unsafe { &*ast };
   let asteroids = unsafe { &*(ast.asteroids as *mut SlotMap<AsteroidRef, Asteroid>) };
   let pos = Vector2::new(x, y);
   match asteroids
      .iter()
      .map(|(k, a)| {
         let ap = a.pos();
         (k, (ap - pos).norm_squared())
      })
      .min_by(|(_, a), (_, b)| a.partial_cmp(b).unwrap_or(std::cmp::Ordering::Equal))
   {
      Some((k, a)) => k.as_ffi(),
      None => AsteroidRef::null().as_ffi(),
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn _asteroid_inField(p: *const Vector2<f64>) -> c_int {
   let p = unsafe { *p };
   match infield(p) {
      Some(v) => v as c_int,
      None => -1,
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn _asteroids_hasCommodity(field: *const naevc::AsteroidAnchor, com: i64) -> c_int {
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
pub extern "C" fn _asteroids_computeInternals(a: *mut naevc::AsteroidAnchor) {
   let a = unsafe { &mut *a };
   a.area = std::f64::consts::PI * a.radius * a.radius;
   a.nmax = (a.area / naevc::ASTEROID_REF_AREA * a.density).round() as i32;
   a.margin = a.maxspeed * a.maxspeed / (4.0 * a.accel) + 50.0;
   a.groupswtotal = 0.0;
   let groupsw = unsafe { array::array_as_slice(a.groupsw) };
   for w in groupsw {
      a.groupswtotal += w;
   }
   if a.asteroids.is_null() {
      let map: SlotMap<AsteroidRef, Asteroid> = SlotMap::with_key();
      a.asteroids = Box::into_raw(Box::new(map)) as *mut naevc::AsteroidVecStorage;
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn _asteroid_hit(
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
      std::ptr::null_mut(),
      dmg,
      std::ptr::null(),
   );
   a.armour -= darmour;
   if a.armour <= 0.0 {
      _asteroid_explode(a, max_rarity, mine_bonus);
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn _asteroid_explode(a: *mut Asteroid, max_rarity: i32, mine_bonus: f64) {
   let a = unsafe { &mut *a };
   let at = &a.atype;

   let dmg = naevc::Damage {
      type_: crate::damagetype::dtype_get(c"explosion_splash".as_ptr()),
      damage: at.damage,
      penetration: at.penetration,
      disable: at.disable,
   };
   unsafe {
      naevc::expl_explode(
         a.solid.pos.x,
         a.solid.pos.y,
         a.solid.vel.x,
         a.solid.vel.y,
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
            .position(Some(a.pos().cast()))
            .velocity(Some(a.pos().cast()))
            .play(true)
            .build();
      }
      Err(e) => warn_err!(e),
   }

   let rad2 = at.alert_range * at.alert_range;
   let la = naevc::LuaAsteroid_t {
      parent: a.parent,
      id: 0, // TODO a.id,
   };
   unsafe {
      naevc::lua_pushasteroid(naevc::naevL, la);
   }
   for p in pilot::get() {
      if (a.pos() - p.pos()).norm_squared() <= rad2 {
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
               let pos =
                  a.pos() + Vector2::new(30.0 * rng::<f64>() - 15.0, 30.0 * rng::<f64>() - 15.0);
               let vel =
                  a.vel() + Vector2::new(20.0 * rng::<f64>() - 10.0, 20.0 * rng::<f64>() - 10.0);
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
      naevc::pilot_untargetAsteroid(a.parent, a.id.as_ffi());
   }

   // Make it respawns
   a.state = State::BgToXx;
   a.timer = 0.0;
   a.timer_max = 0.0;
}

#[unsafe(no_mangle)]
pub extern "C" fn _asteroid_collideQueryIL(
   anc: *mut naevc::AsteroidAnchor,
   il: *mut naevc::IntList,
   x1: c_int,
   y1: c_int,
   x2: c_int,
   y2: c_int,
) {
   let anc = unsafe { &mut *anc };
   unsafe {
      naevc::qt_query(&mut anc.qt, il, x1, y1, x2, y2);
   }
}
