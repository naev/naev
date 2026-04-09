#![allow(dead_code, unused)]
use crate::array;
use crate::commodity::CommodityRef;
use crate::rng::rng;
use anyhow::Context as AnyhowContext;
use anyhow::Result;
use collide::polygon::SpinPolygon;
use helpers::ReferenceC;
use helpers::{binary_search_by_key_ref, sort_by_key_ref};
use naev_core::{nxml, nxml_err_attr_missing, nxml_warn_node_unknown};
use nalgebra::Vector2;
use nlog::{debugx, warn, warn_err};
use rayon::prelude::*;
use renderer::texture::{Texture, TextureBuilder};
use renderer::{Context, ContextWrapper};
use slotmap::SlotMap;
use std::collections::HashMap;
use std::ffi::OsStr;
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
   gfx: Vec<GfxType>,
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
               at.gfx.push(GfxType::Single(gfx))
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

#[derive(Debug)]
pub struct Asteroid {
   id: AsteroidRef,
   parent: i32,
   state: State,
   atype: Arc<Type>,
   gfx: usize,
   armour: f64,

   sol: naevc::Solid,
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

      let r = field.groupswtotal * rng::<f64>();
      let mut wmax = 0.0;
      let groups = unsafe { array::array_as_slice(field.groups) };
      let groupsw = unsafe { array::array_as_slice(field.groupsw) };
      for (g, w) in groups.iter().zip(groupsw.iter()) {
         let g = unsafe { &**g };
         wmax += w;
         if r > wmax {
            continue;
         }

         let wi = 0.0;
         let r = g.wtotal * rng::<f64>();
      }

      /*
      Self {
         parent: field.id,
         state:
      }
      */
      todo!();
   }
}

slotmap::new_key_type! {
pub struct AsteroidRef;
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
   /*
   sort_by_key_ref(&mut data, |at: &Type| &at.name);
   data.windows(2).for_each(|w| {
      if w[0].name == w[1].name {
         warn!("Asteroid Type '{}' loaded twice!", w[0].name);
      }
   });
   */
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
   /*
   sort_by_key_ref(&mut data, |at: &Type| &at.name);
   data.windows(2).for_each(|w| {
      if w[0].name == w[1].name {
         warn!("Asteroid Group '{}' loaded twice!", w[0].name);
      }
   });
   */
   data
}

pub fn load() -> Result<()> {
   #[cfg(debug_assertions)]
   let start = std::time::Instant::now();

   let _ = LazyLock::force(&TYPES);
   let _ = LazyLock::force(&GROUPS);

   // Some debug
   if cfg!(debug_assertions) {
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
   } else {
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

#[unsafe(no_mangle)]
pub extern "C" fn _asteroids_update(dt: f64) {
   update(dt)
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
         for i in 0..ast.nmax {}

         density_max = ast.density.max(density_max);
      }
   }
}
