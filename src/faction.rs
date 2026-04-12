#![allow(dead_code)]
use crate::array::{Array, ArrayCString};
use crate::hook::{HookParam, run_param_deferred};
use crate::nlua::LuaEnv;
use crate::nlua::{NLUA, NLua};
use anyhow::Context as AnyhowContext;
use anyhow::Result;
use gettext::gettext;
use helpers::ReferenceC;
use itertools::Itertools;
use mlua::ErrorContext as MluaContext;
use mlua::{
   BorrowedStr, Either, FromLua, Function, MetaMethod, UserData, UserDataMethods, UserDataRef,
};
use naev_core::{nxml, nxml_err_attr_missing, nxml_warn_node_unknown};
use nlog::{debugx, warn, warn_err, warnx};
use renderer::colour::Colour;
use renderer::{Context, ContextWrapper, colour, texture};
use slotmap::{Key, KeyData, SecondaryMap, SlotMap};
use std::collections::HashMap;
use std::ffi::{CStr, CString, OsStr};
use std::fmt::Debug;
use std::path::{Path, PathBuf};
use std::sync::{Arc, LazyLock, OnceLock};
//#[cfg(not(debug_assertions))]
use std::sync::{Mutex, RwLock};
use tracing::instrument;
//#[cfg(debug_assertions)]
//use tracing_mutex::stdsync::{Mutex, RwLock};

//static_assertions::const_assert_eq!( (1i64<<32) + ((1i64<<32)-1), FactionRef::null() );

#[derive(Copy, Clone, Debug, PartialEq)]
enum GridEntry {
   None,
   Enemies,
   Allies,
   Neutrals,
}
#[derive(Default)]
struct Grid {
   data: Vec<GridEntry>,
   size: usize,
}
impl std::ops::Index<(FactionRef, FactionRef)> for Grid {
   type Output = GridEntry;
   fn index(&self, index: (FactionRef, FactionRef)) -> &Self::Output {
      match &self.data.get(self.offset(index)) {
         Some(v) => v,
         None => {
            warn!("trying to access '{:?}' x '{:?}'", index.0, index.1);
            &GridEntry::None
         }
      }
      //&self.data[self.offset(index)]
   }
}
impl std::ops::IndexMut<(FactionRef, FactionRef)> for Grid {
   fn index_mut(&mut self, index: (FactionRef, FactionRef)) -> &mut Self::Output {
      let offset = self.offset(index);
      &mut self.data[offset]
   }
}
impl Grid {
   const fn new() -> Self {
      Self {
         data: vec![],
         size: 0,
      }
   }

   fn offset(&self, idx: (FactionRef, FactionRef)) -> usize {
      let a = idx.0.slot();
      let b = idx.1.slot();
      if a <= b {
         a * self.size + b
      } else {
         b * self.size + a
      }
   }

   fn recompute(&mut self, factions: &SlotMap<FactionRef, Faction>) -> Result<()> {
      self.size = factions.capacity();
      self.data.clear();
      self.data.resize(self.size * self.size, GridEntry::None);

      for (_, fct) in factions.iter() {
         let dat = &fct.data;
         self[(dat.id, dat.id)] = GridEntry::Allies;
         for a in &dat.allies {
            #[cfg(debug_assertions)]
            {
               let ent = self[(dat.id, *a)];
               if ent != GridEntry::Allies && ent != GridEntry::None {
                  warn!(
                     "Incoherent faction grid! '{}' and '{}' already have contradictory relationships!",
                     &dat.name, &factions[*a].data.name
                  );
               }
            }
            self[(dat.id, *a)] = GridEntry::Allies;
         }
         for e in &dat.enemies {
            self[(dat.id, *e)] = GridEntry::Enemies;
            #[cfg(debug_assertions)]
            {
               let ent = self[(dat.id, *e)];
               if ent != GridEntry::Enemies && ent != GridEntry::None {
                  warn!(
                     "Incoherent faction grid! '{}' and '{}' already have contradictory relationships!",
                     &dat.name, &factions[*e].data.name
                  );
               }
            }
         }
         for n in &dat.neutrals {
            self[(dat.id, *n)] = GridEntry::Neutrals;
            #[cfg(debug_assertions)]
            {
               let ent = self[(dat.id, *n)];
               if ent != GridEntry::Neutrals && ent != GridEntry::None {
                  warn!(
                     "Incoherent faction grid! '{}' and '{}' already have contradictory relationships!",
                     &dat.name, &factions[*n].data.name
                  );
               }
            }
         }
      }
      Ok(())
   }
}
static GRID: RwLock<Grid> = RwLock::new(Grid::new());

/// Full faction data
pub static FACTIONS: LazyLock<RwLock<SlotMap<FactionRef, Faction>>> =
   LazyLock::new(|| RwLock::new(SlotMap::with_key()));
pub static PLAYER: OnceLock<FactionRef> = OnceLock::new();
const PLAYER_FACTION_NAME: &str = "Player";

fn reputation_to_raw(rep: f32) -> f32 {
   let arep = rep.abs();
   (if arep < 10.0 {
      arep
   } else {
      10.0 * 2.0f32.powf(arep / 10.0 - 1.0)
   })
   .copysign(rep)
}

fn raw_to_reputation(raw: f32) -> f32 {
   let araw = raw.abs();
   (if araw < 10.0 {
      araw
   } else {
      10.0 * ((araw / 10.0).log2() + 1.0)
   })
   .copysign(raw)
}

slotmap::new_key_type! {
pub struct FactionRef;
}

impl FactionRef {
   pub fn from_ffi(value: i64) -> Self {
      Self(KeyData::from_ffi(value as u64))
   }

   #[instrument]
   pub fn new(name: &str) -> Option<FactionRef> {
      let factions = FACTIONS.read().unwrap();
      for (id, fct) in factions.iter() {
         if fct.data.name == name {
            return Some(id);
         }
      }
      None
   }

   pub fn with<F, R>(&self, f: F) -> Result<R>
   where
      F: Fn(&Faction) -> R,
   {
      let factions = FACTIONS.read().unwrap();
      match factions.get(*self) {
         Some(fct) => Ok(f(fct)),
         None => anyhow::bail!("faction not found"),
      }
   }

   pub fn with2<F, R>(&self, other: &Self, f: F) -> Result<R>
   where
      F: Fn(&Faction, &Faction) -> R,
   {
      let factions = FACTIONS.read().unwrap();
      if let Some(fct1) = factions.get(*self)
         && let Some(fct2) = factions.get(*other)
      {
         Ok(f(fct1, fct2))
      } else {
         anyhow::bail!("faction not found")
      }
   }

   pub fn with_mut<F, R>(&self, f: F) -> Result<R>
   where
      F: Fn(&mut Faction) -> R,
   {
      let mut factions = match FACTIONS.try_write() {
         Ok(d) => d,
         Err(e) => {
            return Err(anyhow::anyhow!(format!(
               "unable to lock modify factions: {e}"
            )));
         }
      };
      match factions.get_mut(*self) {
         Some(fct) => Ok(f(fct)),
         None => anyhow::bail!("faction not found"),
      }
   }

   #[instrument(skip(self))]
   pub fn hit(&self, val: f32, system: &mlua::Value, source: &str, single: bool) -> Result<f32> {
      let factions = FACTIONS.read().unwrap();
      match factions.get(*self) {
         Some(fct) => {
            let ret = fct.hit_lua(val, system, source, 0, None)?;
            if !single {
               for a in &fct.data.allies {
                  factions[*a].hit_lua(val, system, source, 1, Some(fct))?;
               }
               for e in &fct.data.enemies {
                  factions[*e].hit_lua(-val, system, source, -1, Some(fct))?;
               }
            }
            Ok(ret)
         }
         None => anyhow::bail!("faction not found"),
      }
   }

   fn player_ally(&self, sys: Option<&naevc::StarSystem>) -> bool {
      if let Some(sys) = sys {
         let threshold = self
            .with(|fct| match &fct.api {
               Some(api) => api.friendly_at,
               None => f32::INFINITY,
            })
            .unwrap_or(f32::INFINITY);
         if threshold == f32::INFINITY {
            return false;
         }
         unsafe {
            (naevc::system_getReputationOrGlobal(sys, self.as_ffi()) as f32).round() > threshold
         }
      } else {
         self
            .with(|fct| {
               let std = fct.standing.read().unwrap();
               if let Some(api) = &fct.api {
                  std.player.round() >= api.friendly_at
               } else {
                  false
               }
            })
            .unwrap_or_else(|err| {
               warn_err!(err);
               false
            })
      }
   }

   /// Checks to see if two factions are allies
   pub fn are_allies(&self, other: &Self, sys: Option<&naevc::StarSystem>) -> bool {
      if self == other {
         return true;
      }

      let player_fct = PLAYER.get().unwrap();
      if self == player_fct {
         other.player_ally(sys)
      } else if other == player_fct {
         self.player_ally(sys)
      } else {
         GRID.read().unwrap()[(*self, *other)] == GridEntry::Allies
      }
   }

   fn player_enemy(&self, sys: Option<&naevc::StarSystem>) -> bool {
      if let Some(sys) = sys {
         unsafe { naevc::system_getReputationOrGlobal(sys, self.as_ffi()) < 0.0 }
      } else {
         self
            .with(|fct| fct.standing.read().unwrap().player < 0.)
            .unwrap_or_else(|err| {
               warn_err!(err);
               false
            })
      }
   }

   /// Checks to see if two factions are enemies
   pub fn are_enemies(&self, other: &Self, sys: Option<&naevc::StarSystem>) -> bool {
      if self == other {
         return false;
      }

      let player_fct = PLAYER.get().unwrap();
      if self == player_fct {
         other.player_enemy(sys)
      } else if other == player_fct {
         self.player_enemy(sys)
      } else {
         GRID.read().unwrap()[(*self, *other)] == GridEntry::Enemies
      }
   }

   /// Checks to see if two factions are true neutrals
   pub fn are_neutrals(&self, other: &Self) -> bool {
      if self == other || other == PLAYER.get().unwrap() {
         false
      } else {
         GRID.read().unwrap()[(*self, *other)] == GridEntry::Neutrals
      }
   }
}

#[derive(Debug)]
struct LuaAPI {
   // Scheduler
   sched_env: Option<Arc<LuaEnv>>,
   // Equipping
   equip_env: Option<Arc<LuaEnv>>,
   // Standing Behaviour
   lua_env: LuaEnv,
   friendly_at: f32,
   hit: Function,
   hit_test: Function,
   text_rank: Function,
   text_broad: Function,
   reputation_max: Function,
}
impl LuaAPI {
   fn new(lua: &NLua, data: &FactionData) -> Result<Self> {
      fn new_env(lua: &NLua, script: &str, dir: &str) -> Result<LuaEnv> {
         let mut env = lua.environment_new(script)?;
         if script.is_empty() {
            return Ok(env);
         }
         env.load_standard(lua)?;
         let path = format!("factions//{}/{}.lua", dir, script);
         let data = ndata::read(&path)?;
         let chunk = lua.lua.load(std::str::from_utf8(&data)?).set_name(path);
         env.eval::<()>(lua, chunk)?;
         Ok(env)
      }
      let equip_env = if data.script_equip.is_empty() {
         None
      } else {
         Some(Arc::new(new_env(lua, &data.script_equip, "equip")?))
      };
      let sched_env = if data.script_spawn.is_empty() {
         None
      } else {
         Some(Arc::new(new_env(lua, &data.script_spawn, "spawn")?))
      };
      let lua_env = new_env(lua, &data.script_standing, "standing")?;

      fn load_func(env: &LuaEnv, name: &str) -> Result<Function> {
         match env.get(name) {
            Ok(f) => Ok(f),
            Err(e) => Err(
               e.with_context(|_| {
                  format!(
                     "getting function '{name}' from env '{}'",
                     env.get::<String>("__name").unwrap_or("???".to_string())
                  )
               })
               .into(),
            ),
         }
      }

      if data.script_standing.is_empty() {
         let noop_f32 = lua.lua.create_function(|_, ()| Ok(0.0f32))?;
         let noop_string = lua.lua.create_function(|_, ()| Ok(String::new()))?;
         Ok(LuaAPI {
            equip_env,
            sched_env,
            lua_env,
            friendly_at: f32::INFINITY,
            hit: noop_f32.clone(),
            hit_test: noop_f32.clone(),
            text_rank: noop_string.clone(),
            text_broad: noop_string,
            reputation_max: noop_f32,
         })
      } else {
         let friendly_at = lua_env.get("friendly_at").unwrap_or(70.0);
         let hit = load_func(&lua_env, "hit")?;
         let hit_test = load_func(&lua_env, "hit_test")?;
         let text_rank = load_func(&lua_env, "text_rank")?;
         let text_broad = load_func(&lua_env, "text_broad")?;
         let reputation_max = load_func(&lua_env, "reputation_max")?;
         Ok(LuaAPI {
            equip_env,
            sched_env,
            lua_env,
            friendly_at,
            hit,
            hit_test,
            text_rank,
            text_broad,
            reputation_max,
         })
      }
   }
}

#[derive(Debug)]
struct Standing {
   player: f32,
   p_override: Option<f32>,
   f_known: bool,
   f_invisible: bool,
}

#[derive(Debug)]
pub struct FactionC {
   pub cname: CString,
   clongname: Option<CString>,
   cdisplayname: Option<CString>,
   cmapname: Option<CString>,
   cdescription: CString,
   cai: CString,
   ctags: ArrayCString,
}
impl FactionC {
   fn new(fd: &FactionData) -> Self {
      let cs = |s: &str| CString::new(s).unwrap();
      let cname = cs(&fd.name);
      let clongname = fd.longname.as_ref().map(|s| cs(s));
      let cdisplayname = fd.displayname.as_ref().map(|s| cs(s));
      let cmapname = fd.mapname.as_ref().map(|s| cs(s));
      let cdescription = cs(&fd.description);
      let cai = cs(&fd.ai);
      let ctags = ArrayCString::new(&fd.tags);
      FactionC {
         cname,
         clongname,
         cdisplayname,
         cmapname,
         cdescription,
         cai,
         ctags,
      }
   }
}

#[derive(Debug)]
pub struct Faction {
   api: Option<Arc<LuaAPI>>,
   standing: RwLock<Standing>,
   data: FactionData,

   // C stuff, TODO remove when unnecessary
   pub c: FactionC,
}
impl Faction {
   pub fn player_raw(&self) -> f32 {
      let standing = self.standing.read().unwrap();
      let rep = match standing.p_override {
         Some(std) => std,
         None => standing.player,
      };
      reputation_to_raw(rep)
   }

   pub fn player(&self) -> f32 {
      let standing = self.standing.read().unwrap();
      match standing.p_override {
         Some(std) => std,
         None => standing.player,
      }
   }

   pub fn set_player_raw(&self, raw: f32) {
      self.set_player(raw_to_reputation(raw))
   }

   #[instrument(skip(self))]
   pub fn set_player(&self, std: f32) {
      let mut standing = self.standing.write().unwrap();
      if standing.p_override.is_none() {
         let delta = std - standing.player;
         standing.player = std;
         for sys in crate::system::get_mut() {
            for sp in sys.presence_mut() {
               if FactionRef::from_ffi(sp.faction) != self.data.id {
                  continue;
               }
               sp.local = std.into();
            }
         }

         let hparam = [
            HookParam::Faction(self.data.id),
            HookParam::Nil,
            HookParam::Number(delta as f64),
            HookParam::String(c"script"),
            HookParam::Number(0.0),
            HookParam::Nil,
         ];
         run_param_deferred("standing", &hparam);
      }
   }

   pub fn r#override(&self) -> Option<f32> {
      let standing = self.standing.read().unwrap();
      standing.p_override
   }

   pub fn set_override(&self, std: Option<f32>) {
      let mut standing = self.standing.write().unwrap();
      standing.p_override = std;
   }

   pub fn known(&self) -> bool {
      self.standing.read().unwrap().f_known
   }

   pub fn set_known(&self, state: bool) {
      self.standing.write().unwrap().f_known = state;
   }

   pub fn invisible(&self) -> bool {
      self.standing.read().unwrap().f_invisible
   }

   pub fn set_invisible(&self, state: bool) {
      self.standing.write().unwrap().f_invisible = state;
   }

   pub fn fixed(&self) -> bool {
      self.data.f_static
   }

   pub fn dynamic(&self) -> bool {
      self.data.f_dynamic
   }

   pub fn add_enemy(&mut self, other: FactionRef) {
      let player = *PLAYER.get().unwrap();
      if other == player || self.data.id == player {
         warn!("can't add player as enemy, set player reputation instead");
         return;
      }
      if !self.data.enemies.contains(&other) {
         self.data.enemies.push(other);
      }
   }

   pub fn remove_enemy(&mut self, other: FactionRef) {
      let lst = &mut self.data.enemies;
      if let Some(p) = lst.iter().position(|f| other == *f) {
         lst.swap_remove(p);
      }
   }

   pub fn add_neutral(&mut self, other: FactionRef) {
      let player = *PLAYER.get().unwrap();
      if other == player || self.data.id == player {
         warn!("can't add player as neutral, set player reputation instead");
         return;
      }
      if !self.data.neutrals.contains(&other) {
         self.data.neutrals.push(other);
      }
   }

   pub fn remove_neutral(&mut self, other: FactionRef) {
      let lst = &mut self.data.neutrals;
      if let Some(p) = lst.iter().position(|f| other == *f) {
         lst.swap_remove(p);
      }
   }

   pub fn add_ally(&mut self, other: FactionRef) {
      let player = *PLAYER.get().unwrap();
      if other == player || self.data.id == player {
         warn!("can't add player as ally, set player reputation instead");
         return;
      }
      if !self.data.allies.contains(&other) {
         self.data.allies.push(other);
      }
   }

   pub fn remove_ally(&mut self, other: FactionRef) {
      let lst = &mut self.data.allies;
      if let Some(p) = lst.iter().position(|f| other == *f) {
         lst.swap_remove(p);
      }
   }

   fn reputation_max(&self) -> Result<f32> {
      if let Some(api) = &self.api {
         Ok(api.lua_env.call(&NLUA, &api.reputation_max, ())?)
      } else {
         anyhow::bail!(
            "reputation_max function not defined for faction '{}'",
            &self.data.name
         )
      }
   }

   fn text_rank(&self, value: Option<f32>) -> Result<String> {
      if let Some(api) = &self.api {
         let value = value.unwrap_or(self.player());
         Ok(api.lua_env.call::<String>(&NLUA, &api.text_rank, value)?)
      } else {
         Ok(String::from("???"))
      }
   }

   fn hit_lua(
      &self,
      val: f32,
      system: &mlua::Value,
      source: &str,
      secondary: i32,
      parent: Option<&Faction>,
   ) -> Result<f32> {
      if self.data.f_static {
         return Ok(0.);
      } else {
         let std = self.standing.read().unwrap();
         if std.p_override.is_some() {
            return Ok(0.);
         }
      }
      if let Some(api) = &self.api {
         let delta: f32 = api.lua_env.call(
            &NLUA,
            &api.hit,
            (system, val, source, secondary, parent.map(|f| f.data.id)),
         )?;
         if delta.abs() > 1e-3 {
            let hsys = if system.is_nil() {
               HookParam::Nil
            } else {
               let sysid = crate::system::from_lua_index(&NLUA.lua, system)?;
               HookParam::Ssys(sysid)
            };
            let hparent = if let Some(parent) = parent {
               HookParam::Faction(parent.data.id)
            } else {
               HookParam::Nil
            };
            let source = CString::new(source)?;
            let hparam = [
               HookParam::Faction(self.data.id),
               hsys,
               HookParam::Number(delta as f64),
               HookParam::StringFree(unsafe { naevc::strdup(source.as_ptr()) }),
               HookParam::Number(secondary as f64),
               hparent,
            ];
            run_param_deferred("standing", &hparam);
         }
         unsafe { naevc::space_factionChange() };
         Ok(delta)
      } else {
         anyhow::bail!("hit function not defined for faction '{}'", &self.data.name)
      }
   }

   fn hit_test_lua(
      &self,
      val: f32,
      system: &mlua::Value,
      source: &str,
      secondary: bool,
   ) -> Result<f32> {
      if self.data.f_static {
         return Ok(0.);
      } else {
         let std = self.standing.read().unwrap();
         if std.p_override.is_some() {
            return Ok(0.);
         }
      }
      if let Some(api) = &self.api {
         Ok(api
            .lua_env
            .call(&NLUA, &api.hit_test, (system, val, source, secondary))?)
      } else {
         anyhow::bail!(
            "hit_test function not defined for faction '{}'",
            &self.data.name
         )
      }
   }
}

#[derive(Debug, Clone)]
pub struct Generator {
   /// ID of faction to generate
   id: FactionRef,
   /// Weight modifier
   weight: f32,
}

#[derive(Debug, Default)]
struct FactionLoad {
   // Generators
   generator: Vec<(String, f32)>,

   // Relationships
   enemies: Vec<String>,
   allies: Vec<String>,
   neutrals: Vec<String>,
}

#[derive(Debug, Default)]
pub struct FactionData {
   id: FactionRef,
   pub name: String,
   pub longname: Option<String>,
   pub displayname: Option<String>,
   pub mapname: Option<String>,
   pub ai: String,
   pub description: String,
   pub local_th: f32,

   // Scripts
   script_standing: String,
   script_spawn: String,
   script_equip: String,

   // Graphics
   pub logo: Option<texture::Texture>,
   pub colour: Colour,

   // Relationships
   pub enemies: Vec<FactionRef>,
   pub allies: Vec<FactionRef>,
   pub neutrals: Vec<FactionRef>,

   // Player stuff
   pub player_def: f32,

   // Safe lanes
   pub lane_length_per_presence: f32,
   pub lane_base_cost: f32,

   // Presence
   pub generators: Vec<Generator>,

   // Flags
   pub f_static: bool,
   pub f_invisible: bool,
   pub f_known: bool,
   pub f_dynamic: bool,
   pub f_useshiddenjumps: bool,

   // Tags
   pub tags: Vec<String>,
}
impl FactionData {
   /// Loads the elementary faction stuff, does not fill out information dependent on other
   /// factions
   #[instrument(skip(ctx))]
   fn new<P: AsRef<Path> + Debug>(
      ctx: &ContextWrapper,
      filename: P,
   ) -> Result<(Self, FactionLoad)> {
      let mut fctload = FactionLoad::default();
      let mut fct = FactionData {
         local_th: 10.0,
         colour: colour::WHITE,
         ..Default::default()
      };

      let data = ndata::read(filename)?;
      let doc = roxmltree::Document::parse(std::str::from_utf8(&data)?)?;
      let root = doc.root_element();
      fct.name = String::from(match root.attribute("name") {
         Some(n) => n,
         None => {
            return nxml_err_attr_missing!("Damage Type", "name");
         }
      });

      for node in root.children() {
         if !node.is_element() {
            continue;
         }
         match node.tag_name().name().to_lowercase().as_str() {
            "player" => fct.player_def = nxml::node_f32(node)?,
            "longname" => {
               fct.longname = Some(nxml::node_string(node)?);
            }
            "display" => {
               fct.displayname = Some(nxml::node_string(node)?);
            }
            "mapname" => {
               fct.mapname = Some(nxml::node_string(node)?);
            }
            "description" => {
               fct.description = nxml::node_string(node)?;
            }
            "ai" => {
               fct.ai = nxml::node_string(node)?;
            }
            "local_th" => fct.local_th = nxml::node_f32(node)?,
            "lane_length_per_presence" => fct.lane_length_per_presence = nxml::node_f32(node)?,
            "lane_base_cost" => fct.lane_base_cost = nxml::node_f32(node)?,
            "colour" => {
               if let Some(r) = node.attribute("r")
                  && let Some(g) = node.attribute("g")
                  && let Some(b) = node.attribute("b")
               {
                  fct.colour = Colour::from_gamma(r.parse()?, g.parse()?, b.parse()?);
               } else {
                  fct.colour = Colour::from_name(nxml::node_str(node)?).unwrap_or(colour::WHITE)
               }
            }
            "logo" => {
               let gfxname = nxml::node_texturepath(node, "gfx/logo/")?;
               fct.logo = Some(
                  texture::TextureBuilder::new()
                     .path(&gfxname)
                     .build_wrap(ctx)?,
               );
            }
            "known" => fct.f_known = true,
            "static" => fct.f_static = true,
            "invisible" => fct.f_invisible = true,
            "useshiddenjumps" => fct.f_useshiddenjumps = true,
            "standing" => fct.script_standing = nxml::node_string(node)?,
            "spawn" => fct.script_spawn = nxml::node_string(node)?,
            "equip" => fct.script_equip = nxml::node_string(node)?,
            "tags" => {
               for node in node.children() {
                  if !node.is_element() {
                     continue;
                  }
                  if let Some(t) = node.text() {
                     fct.tags.push(String::from(t));
                  }
               }
            }
            // Temporary scaoffolding stuff
            "allies" => {
               for node in node.children() {
                  if !node.is_element() {
                     continue;
                  }
                  fctload.allies.push(nxml::node_string(node)?);
               }
            }
            "enemies" => {
               for node in node.children() {
                  if !node.is_element() {
                     continue;
                  }
                  fctload.enemies.push(nxml::node_string(node)?);
               }
            }
            "neutrals" => {
               for node in node.children() {
                  if !node.is_element() {
                     continue;
                  }
                  fctload.neutrals.push(nxml::node_string(node)?);
               }
            }
            "generator" => {
               let weight = match node.attribute("weight") {
                  Some(str) => str.parse::<f32>()?,
                  None => 1.0,
               };
               fctload.generator.push((nxml::node_string(node)?, weight));
            }

            // Case we missed everything
            tag => nxml_warn_node_unknown!("Faction", &fct.name, tag),
         }
      }

      Ok((fct, fctload))
   }

   /// Checks to see if two factions are allies
   fn are_allies(&self, other: &Self) -> bool {
      for a in &self.allies {
         if other.id == *a {
            return true;
         }
      }
      for a in &other.allies {
         if self.id == *a {
            return true;
         }
      }
      false
   }

   /// Checks to see if two factions are enemies
   fn are_enemies(&self, other: &Self) -> bool {
      for a in &self.enemies {
         if other.id == *a {
            return true;
         }
      }
      for a in &other.enemies {
         if self.id == *a {
            return true;
         }
      }
      false
   }

   /// Checks to see if two factions are truly neutral to each other
   fn are_neutrals(&self, other: &Self) -> bool {
      for a in &self.neutrals {
         if other.id == *a {
            return true;
         }
      }
      for a in &other.neutrals {
         if self.id == *a {
            return true;
         }
      }
      false
   }

   fn shortname(&self) -> &str {
      gettext(self.displayname.as_ref().unwrap_or(&self.name))
   }

   fn longname(&self) -> &str {
      gettext(
         self
            .longname
            .as_ref()
            .unwrap_or(self.displayname.as_ref().unwrap_or(&self.name)),
      )
   }

   fn mapname(&self) -> &str {
      gettext(
         self
            .mapname
            .as_ref()
            .unwrap_or(self.displayname.as_ref().unwrap_or(&self.name)),
      )
   }
}

/// Loads all the Data
#[instrument]
pub fn load() -> Result<()> {
   // Since we hardcode this C side, we have to make sure it is in-fact correct.
   // Not static, so we have to do it runtime at the moment.
   assert_eq!(
      i64::from_ne_bytes(naevc::FACTION_NULL.to_ne_bytes()),
      FactionRef::null().as_ffi()
   );

   #[cfg(debug_assertions)]
   let start = std::time::Instant::now();

   let ctx = Context::get().as_safe_wrap();
   let base: PathBuf = "factions/".into();
   let files: Vec<_> = ndata::read_dir(&base)?
      .into_iter()
      .filter(|filename| filename.extension() == Some(OsStr::new("xml")))
      .collect();
   let mut data = FACTIONS.write().unwrap();
   let mut load = SecondaryMap::new();
   let mut fctmap: HashMap<String, FactionRef> = HashMap::new();

   // Add play faction before parsing files
   std::iter::once((
      FactionData {
         name: String::from(PLAYER_FACTION_NAME),
         displayname: Some(String::from("Escort")),
         f_static: true,
         f_invisible: true,
         ai: "player".to_string(),
         ..Default::default()
      },
      FactionLoad::default(),
   ))
   .chain(
      files
         //.par_iter()
         .iter()
         .filter_map(
            |filename| match FactionData::new(&ctx, base.join(filename)) {
               Ok(sp) => Some(sp),
               Err(e) => {
                  warn!("Unable to load Faction '{}': {e}", filename.display());
                  None
               }
            },
         ),
   )
   .for_each(|(mut fd, fl)| {
      let name = fd.name.clone();
      let id = data.insert_with_key(|k| {
         fd.id = k;
         Faction {
            api: None,
            standing: RwLock::new(Standing {
               player: fd.player_def,
               p_override: None,
               f_known: fd.f_known,
               f_invisible: fd.f_invisible,
            }),
            c: FactionC::new(&fd),
            data: fd,
         }
      });
      load.insert(id, fl);
      fctmap.insert(name, id);
   });

   // Seconda pass, social and generators
   for (id, fd) in data.iter_mut() {
      let fl = load.get(id).unwrap();
      // Load generators
      for (name, weight) in &fl.generator {
         if let Some(id) = fctmap.get(name) {
            fd.data.generators.push(Generator {
               id: *id,
               weight: *weight,
            });
         }
      }
      // Load social
      for name in &fl.enemies {
         if let Some(id) = fctmap.get(name) {
            fd.data.enemies.push(*id);
         }
      }
      for name in &fl.allies {
         if let Some(id) = fctmap.get(name) {
            fd.data.allies.push(*id);
         }
      }
      for name in &fl.neutrals {
         if let Some(id) = fctmap.get(name) {
            fd.data.neutrals.push(*id);
         }
      }
   }

   // Check duplicates
   for name in data.iter().map(|(_, f)| &f.data.name).duplicates().unique() {
      warn!("Faction '{name}' is duplicated!");
   }

   // Compute grid
   GRID.write().unwrap().recompute(&data)?;

   // Some debug
   #[cfg(debug_assertions)]
   {
      let n = data.len();
      debugx!(
         gettext::ngettext(
            "Loaded {} Faction in {:.3} s",
            "Loaded {} Factions in {:.3} s",
            n as u64
         ),
         n,
         start.elapsed().as_secs_f32()
      );
   }
   #[cfg(not(debug_assertions))]
   {
      let n = data.len();
      debugx!(
         gettext::ngettext("Loaded {} Faction", "Loaded {} Factions", n as u64),
         n
      );
   }

   // Save the data
   drop(data);
   match FactionRef::new(PLAYER_FACTION_NAME) {
      Some(id) => PLAYER.set(id).unwrap_or_else(|_| {
         warn!("unable to set player faction ID");
      }),
      None => unreachable!(),
   };

   Ok(())
}

/// Load the Lua scripts, needs to be run after most things are loaded up
pub fn load_lua() -> Result<()> {
   let lua = &NLUA;

   // Load the APIs
   let apis = FACTIONS
      .read()
      .unwrap()
      .iter()
      .filter_map(|(id, fct)| {
         let api = LuaAPI::new(lua, &fct.data)
            .with_context(|| format!("initializing Lua for faction '{}'", fct.data.name));
         match api {
            Ok(api) => Some((id, api)),
            Err(e) => {
               warn_err!(e);
               None
            }
         }
      })
      .collect::<Vec<_>>();

   // Write out
   let mut data = FACTIONS.write().unwrap();
   for (id, api) in apis {
      data.get_mut(id).unwrap().api = Some(api.into());
   }
   Ok(())
}

impl FromLua for FactionRef {
   fn from_lua(value: mlua::Value, _: &mlua::Lua) -> mlua::Result<Self> {
      match value {
         Value::UserData(ud) => Ok(*ud.borrow::<Self>()?),
         Value::String(name) => {
            let name = &name.to_str()?;
            Ok(FactionRef::new(name).with_context(|| format!("faction '{name}' not found"))?)
         }
         val => Err(mlua::Error::RuntimeError(format!(
            "unable to convert {} to FactionRef",
            val.type_name()
         ))),
      }
   }
}

/*@
 * @brief Lua bindings to deal with factions.
 *
 * Use like:
 * @code
 * f = faction.get( "Empire" )
 * if f:playerStanding() < 0 then
 *    -- player is hostile to Empire
 * end
 * @endcode
 *
 * @luamod faction
 */
impl UserData for FactionRef {
   fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
      /*@
       * @brief Gets the faction's translated short name.
       *
       * @note Equivalent to faction:name()
       *
       *    @luatparam Faction f The faction to get the name of.
       *    @luatreturn string The name of the faction.
       * @luafunc __tostring
       */
      methods.add_meta_function(MetaMethod::ToString, |_, this: Self| {
         Ok(this.with(|fct| fct.data.shortname().to_string())?)
      });
      /*@
       * @brief equality (`__eq()`) metamethod for factions.
       *
       * You can use the `==` operator within Lua to compare factions with this.
       *
       * @usage if f == faction.get( "Dvaered" ) then
       *
       *    @luatparam Faction f Faction comparing.
       *    @luatparam Faction comp faction to compare against.
       *    @luatreturn boolean true if both factions are the same.
       * @luafunc __eq
       */
      methods.add_meta_function(
         MetaMethod::Eq,
         |_,
          (this, other): (UserDataRef<FactionRef>, UserDataRef<FactionRef>)|
          -> mlua::Result<bool> { Ok(*this == *other) },
      );
      /*@
       * @brief Adds a raw value to a reputation resulting in a reputation value.
       *
       *    @luatparam number base Base reputation to add to.
       *    @luatparam number term Raw term to add.
       *    @luatreturn number The added value in reputation space.
       * @luafunc rep_add
       */
      methods.add_function(
         "rep_add",
         |_, (base, term): (f32, f32)| -> mlua::Result<f32> {
            Ok(raw_to_reputation(reputation_to_raw(base) + term))
         },
      );
      methods.add_function("rep_to_raw", |_, rep: f32| -> mlua::Result<f32> {
         Ok(reputation_to_raw(rep))
      });
      methods.add_function("raw_to_rep", |_, raw: f32| -> mlua::Result<f32> {
         Ok(raw_to_reputation(raw))
      });
      /*@
       * @brief Gets a faction if it exists.
       *
       * @usage f = faction.exists( "Empire" )
       *
       *    @luatparam string name Name of the faction to get if exists.
       *    @luatreturn Faction The faction matching name or nil if not matched.
       * @luafunc exists
       */
      methods.add_function(
         "exists",
         |_, name: BorrowedStr| -> mlua::Result<Option<Self>> {
            for (id, fct) in FACTIONS.read().unwrap().iter() {
               if name == fct.data.name {
                  return Ok(Some(id));
               }
            }
            Ok(None)
         },
      );
      /*@
       * @brief Gets the faction based on its name.
       *
       * @usage f = faction.get( "Empire" )
       *
       *    @luatparam string name Name of the faction to get.
       *    @luatreturn Faction The faction matching name.
       * @luafunc get
       */
      methods.add_function(
         "get",
         |_, name: Either<UserDataRef<FactionRef>, BorrowedStr>| -> mlua::Result<Self> {
            match name {
               Either::Left(fr) => Ok(*fr),
               Either::Right(name) => {
                  for (id, fct) in FACTIONS.read().unwrap().iter() {
                     if name == fct.data.name {
                        return Ok(id);
                     }
                  }
                  Err(mlua::Error::RuntimeError(format!(
                     "Faction '{name}' not found."
                  )))
               }
            }
         },
      );
      /*@
       * @brief Gets all the factions.
       *
       *    @luatreturn {Faction,...} An ordered table containing all of the factions.
       * @luafunc getAll
       */
      methods.add_function("getAll", |_, ()| -> mlua::Result<Vec<Self>> {
         Ok(FACTIONS.read().unwrap().keys().collect())
      });
      /*@
       * @brief Gets the player's faction.
       *
       * @usage pf = faction.player()
       *
       *    @luareturn Faction The player's faction.
       * @luafunc player
       */
      methods.add_function("player", |_, ()| -> mlua::Result<Self> {
         Ok(*PLAYER.get().unwrap())
      });
      /*@
       * @brief Gets the faction's translated short name.
       *
       * This translated name should be used for display purposes (e.g.
       * messages) where the shorter version of the faction's display name
       * should be used. It cannot be used as an identifier for the faction;
       * for that, use faction.nameRaw() instead.
       *
       * @usage shortname = f:name()
       *
       *    @luatparam Faction f The faction to get the name of.
       *    @luatreturn string The name of the faction.
       * @luafunc name
       */
      methods.add_method("name", |_, this, ()| -> mlua::Result<String> {
         Ok(this.with(|fct| fct.data.shortname().to_string())?)
      });
      /*@
       * @brief Gets the faction's raw / "real" (untranslated, internal) name.
       *
       * This untranslated name should be used for identification purposes
       * (e.g. can be passed to faction.get()). It should not be used for
       * display purposes; for that, use faction.name() or faction.longname()
       * instead.
       *
       * @usage name = f:nameRaw()
       *
       *    @luatparam Faction f The faction to get the name of.
       *    @luatreturn string The name of the faction.
       * @luafunc nameRaw
       */
      methods.add_method("nameRaw", |_, this, ()| -> mlua::Result<String> {
         Ok(this.with(|fct| fct.data.name.clone())?)
      });
      /*@
       * @brief Gets the faction's translated long name.
       *
       * This translated name should be used for display purposes (e.g.
       * messages) where the longer version of the faction's display name
       * should be used. It cannot be used as an identifier for the faction;
       * for that, use faction.nameRaw() instead.
       *
       * @usage longname = f:longname()
       *    @luatparam Faction f Faction to get long name of.
       *    @luatreturn string The long name of the faction (translated).
       * @luafunc longname
       */
      methods.add_method("longname", |_, this, ()| -> mlua::Result<String> {
         Ok(this.with(|fct| fct.data.longname().to_string())?)
      });
      /*@
       * @brief Checks to see if two factions are truly neutral with respect to each
       * other.
       *
       *    @luatparam Faction f Faction to check against.
       *    @luatparam Faction n Faction to check if is true neutral.
       *    @luatreturn boolean true if they are truly neutral, false if they aren't.
       * @luafunc areNeutral
       */
      methods.add_function(
         "areNeutral",
         |_, (this, other): (FactionRef, FactionRef)| -> mlua::Result<bool> {
            Ok(this.with2(&other, |fct1, fct2| fct1.data.are_neutrals(&fct2.data))?)
         },
      );
      /*@
       * @brief Checks to see if f is an enemy of e.
       *
       * @usage if f:areEnemies( faction.get( "Dvaered" ) ) then
       *
       *    @luatparam Faction f Faction to check against.
       *    @luatparam Faction e Faction to check if is an enemy.
       *    @luatparam[opt] System sys System to check to see if they are enemies in.
       * Mainly for when comparing to the player's faction.
       *    @luatreturn boolean true if they are enemies, false if they aren't.
       * @luafunc areEnemies
       */
      methods.add_function(
         "areEnemies",
         |lua,
          (this, other, sys): (FactionRef, FactionRef, Option<mlua::Value>)|
          -> mlua::Result<bool> {
            if let Some(sys) = sys {
               let sys = crate::system::from_lua(lua, &sys)?;
               Ok(this.are_enemies(&other, Some(sys)))
            } else {
               Ok(this.are_enemies(&other, None))
            }
         },
      );
      /*@
       * @brief Checks to see if f is an ally of a.
       *
       * @usage if f:areAllies( faction.get( "Pirate" ) ) then
       *
       *    @luatparam Faction f Faction to check against.
       *    @luatparam faction a Faction to check if is an enemy.
       *    @luatparam[opt] System sys System to check to see if they are allies in.
       * Mainly for when comparing to the player's faction.
       *    @luatreturn boolean true if they are enemies, false if they aren't.
       * @luafunc areAllies
       */
      methods.add_function(
         "areAllies",
         |lua,
          (this, other, sys): (FactionRef, FactionRef, Option<mlua::Value>)|
          -> mlua::Result<bool> {
            if let Some(sys) = sys {
               let sys = crate::system::from_lua(lua, &sys)?;
               Ok(this.are_allies(&other, Some(sys)))
            } else {
               Ok(this.are_allies(&other, None))
            }
         },
      );

      /*@
       * @brief Modifies the player's standing with the faction.
       *
       * Also can modify the standing with allies and enemies of the faction.
       *
       * @usage f:hit( -5, system.cur() ) -- Lowers faction by 5 at the current system
       * @usage f:hit( 10 ) -- Globally increases faction by 10
       * @usage f:hit( 10, nil, nil, true ) -- Globally increases faction by 10, but
       * doesn't affect allies nor enemies of the faction.
       *
       *    @luatparam Faction f Faction to modify player's standing with.
       *    @luatparam number mod Amount of reputation to change.
       *    @luatparam System|nil extent Whether to make the faction hit local at a
       * system, or global affecting all systems of the faction.
       *    @luatparam[opt="script"] string reason Reason behind it. This is passed as
       * a string to the faction `hit` function. The engine can generate `destroy` and
       * `distress` sources. For missions the default is `script`.
       *    @luatparam[opt=false] boolean ignore_others Whether or not the hit should
       * affect allies/enemies of the faction getting a hit.
       *    @luatreturn How much the reputation was actually changed after Lua script
       * was run.
       * @luafunc hit
       */
      methods.add_function(
         "hit",
         |_,
          (this, modifier, extent, reason, ignore_others): (
            FactionRef,
            f32,
            mlua::Value,
            Option<BorrowedStr>,
            Option<bool>,
         )|
          -> mlua::Result<f32> {
            let reason = reason.as_ref().map_or("script", |v| v);
            let ignore_others = ignore_others.unwrap_or(false);
            Ok(this.hit(modifier, &extent, reason, ignore_others)?)
         },
      );
      /*@
       * @brief Simulates modifying the player's standing with a faction and computes
       * how much would be changed.
       *
       *    @luatparam Faction f Faction to simulate player's standing with.
       *    @luatparam number mod Amount of reputation to simulate change.
       *    @luatparam System|nil extent Whether to make the faction hit local at a
       * system, or global.
       *    @luatparam[opt="script"] string reason Reason behind it. This is passed as
       * a string to the faction `hit` function. The engine can generate `destroy` and
       * `distress` sources. For missions the default is `script`.
       *    @luatreturn How much the reputation was actually changed after Lua script
       * was run.
       * @luafunc hitTest
       */
      methods.add_function(
         "hitTest",
         |_,
          (this, modifier, extent, reason): (FactionRef, f32, mlua::Value, Option<BorrowedStr>)|
          -> mlua::Result<f32> {
            let reason = reason.as_ref().map_or("script", |v| v);
            let ignore_others = true;
            Ok(this.with(|fct| fct.hit_test_lua(modifier, &extent, reason, ignore_others))??)
         },
      );
      /*@
       * @brief Gets the player's global reputation with the faction.
       *
       * @usage if f:reputationGlobal() >= 0 then -- Player is not hostile
       *
       *    @luatparam Faction f Faction to get player's standing with.
       *    @luatreturn number The value of the standing.
       * @luafunc reputationGlobal
       */
      methods.add_function(
         "reputationGlobal",
         |_, this: FactionRef| -> mlua::Result<f32> { Ok(this.with(|fct| fct.player())?) },
      );
      /*@
       * @brief Gets the human readable standing text corresponding (translated).
       *
       *    @luatparam faction f Faction to get standing text from.
       *    @luatparam[opt=f:reputationGlobal()] number|nil val Value to get the
       * standing text of, or nil to use the global faction standing.
       *    @luatreturn string Translated text corresponding to the faction value.
       * @luafunc reputationText
       */
      methods.add_function(
         "reputationText",
         |_, (this, value): (FactionRef, Option<f32>)| -> mlua::Result<String> {
            Ok(this.with(|fct| fct.text_rank(value))??)
         },
      );
      /*@
       * @brief Gets the player's default reputation with the faction.
       *
       *    @luatparam Faction f Faction to get player's default standing with.
       *    @luatreturn number The value of the standing.
       * @luafunc reputationDefault
       */
      methods.add_method("reputationDefault", |_, this, ()| -> mlua::Result<f32> {
         Ok(this.with(|fct| fct.data.player_def)?)
      });
      /*@
       * @brief Overrides the player's faction global standing with a faction. Use
       * sparingly as it overwrites local standings at all systems.
       *
       *    @luatparam Faction f Faction to set the player's global reputation with.
       *    @luatparam number The value of the reputation to set to.
       * @luafunc setReputationGlobal
       */
      methods.add_method(
         "setReputationGlobal",
         |_, this, value: f32| -> mlua::Result<()> { Ok(this.with(|fct| fct.set_player(value))?) },
      );
      /*@
       * @brief Enforces the local threshold of a faction starting at a particular
       * system. Meant to be used when computing faction hits from the faction
       * standing Lua scripts. Not meant for use elsewhere.
       *
       *    @luatparam Faction f Faction to apply local threshold to.
       *    @luatparam System sys System to compute the threshold from. This will
       * be the reference and will not have its value modified.
       * @luafunc applyLocalThreshold
       */
      methods.add_method(
         "applyLocalThreshold",
         |lua, this, sys: mlua::Value| -> mlua::Result<()> {
            let systems = crate::system::get_mut();
            let sysid = crate::system::from_lua_index(lua, &sys)? as usize;
            let sys = unsafe { naevc::system_getIndex(sysid as i32) };
            let (th, usehidden) =
               this.with(|fct| (fct.data.local_th as f64, fct.data.f_useshiddenjumps))?;
            let fid = this.as_ffi();

            let srep = unsafe { naevc::system_getFactionPresence(sys, fid) };
            if srep.is_null() {
               return Ok(());
            }
            let srep = unsafe { &*srep };
            if srep.value <= 0. {
               return Ok(());
            }

            let rep = srep.local;
            let mut n = 0.0;
            let mut done = Vec::from([sysid]);
            let mut queuea = Vec::from([sysid]);
            let mut queueb = Vec::<usize>::new();
            // We want to expand out one jump at a time
            while !queuea.is_empty() {
               // Clear first queue
               for i in queuea.drain(..) {
                  let qsys = &mut systems[i];

                  let srep = unsafe { naevc::system_getFactionPresence(qsys.as_ptr_mut(), fid) };
                  if !srep.is_null() && th.is_finite() {
                     let srep = unsafe { &mut *srep };
                     srep.local = srep.local.clamp(rep - n * th, rep + n * th);
                  }

                  for j in qsys.jumps() {
                     if (j.flags & naevc::JP_HIDDEN) > 0 && !usehidden {
                        continue;
                     }

                     let nsys = unsafe { &*j.target };
                     let nsysid = nsys.id as usize;
                     if done.contains(&nsysid) {
                        continue;
                     }

                     if unsafe { naevc::system_getPresence(nsys, fid) } > 0. {
                        queueb.push(nsysid);
                     }
                     done.push(nsysid);
                  }
               }
               // Add jump, swap buffers, and expand again
               n += 1.0;
               (queuea, queueb) = (queueb, queuea);
            }
            Ok(())
         },
      );
      /*@
       * @brief Gets the enemies of the faction.
       *
       * @usage for k,v in pairs(f:enemies()) do -- Iterates over enemies
       *
       *    @luatparam Faction f Faction to get enemies of.
       *    @luatreturn {Faction,...} A table containing the enemies of the faction.
       * @luafunc enemies
       */
      methods.add_method("enemies", |_, this, ()| -> mlua::Result<Vec<Self>> {
         Ok(this.with(|fct| fct.data.enemies.clone())?)
      });
      /*@
       * @brief Gets the allies of the faction.
       *
       * @usage for k,v in pairs(f:allies()) do -- Iterate over faction allies
       *
       *    @luatparam Faction f Faction to get allies of.
       *    @luatreturn {Faction,...} A table containing the allies of the faction.
       * @luafunc allies
       */
      methods.add_method("allies", |_, this, ()| -> mlua::Result<Vec<Self>> {
         Ok(this.with(|fct| fct.data.allies.clone())?)
      });
      /*@
       * @brief Gets whether or not a faction uses hidden jumps.
       *
       *    @luatparam Faction f Faction to get whether or not they use hidden jumps.
       *    @luatreturn boolean true if the faction uses hidden jumps, false
       * otherwise.
       * @luafunc usesHiddenJumps
       */
      methods.add_method("usesHiddenJumps", |_, this, ()| -> mlua::Result<bool> {
         Ok(this.with(|fct| fct.data.f_useshiddenjumps)?)
      });
      /*@
       * @brief Gets the faction logo.
       *
       *    @luatparam Faction f Faction to get logo from.
       *    @luatreturn Tex The faction logo or nil if not applicable.
       * @luafunc logo
       */
      methods.add_method(
         "logo",
         |_, this, ()| -> mlua::Result<Option<texture::Texture>> {
            Ok(this
               .with(|fct| fct.data.logo.as_ref().map(|t| t.try_clone()))?
               .transpose()?)
         },
      );
      /*@
       * @brief Gets the faction colour.
       *
       *    @luatparam Faction f Faction to get colour from.
       *    @luatreturn Colour|nil The faction colour or nil if not applicable.
       * @luafunc colour
       */
      methods.add_method("colour", |_, this, ()| -> mlua::Result<Colour> {
         Ok(this.with(|fct| fct.data.colour)?)
      });
      /*@
       * @brief Checks to see if a faction is known by the player.
       *
       * @usage b = f:known()
       *
       *    @luatparam Faction f Faction to check if the player knows.
       *    @luatreturn boolean true if the player knows the faction.
       * @luafunc known
       */
      methods.add_method("known", |_, this, ()| -> mlua::Result<bool> {
         Ok(this.with(|fct| fct.known())?)
      });
      /*@
       * @brief Sets a faction's known state.
       *
       * @usage f:setKnown( false ) -- Makes faction unknown.
       *    @luatparam Faction f Faction to set known.
       *    @luatparam[opt=false] boolean b Whether or not to set as known.
       * @luafunc setKnown
       */
      methods.add_method_mut("setKnown", |_, this, known: bool| -> mlua::Result<()> {
         Ok(this.with(|fct| fct.set_known(known))?)
      });
      /*@
       * @brief Checks to see if a faction is invisible the player.
       *
       * @usage b = f:invisible()
       *
       *    @luatparam Faction f Faction to check if is invisible to the player.
       *    @luatreturn boolean true if the faction is invisible to the player.
       * @luafunc invisible
       */
      methods.add_method("invisible", |_, this, ()| -> mlua::Result<bool> {
         Ok(this.with(|fct| fct.invisible())?)
      });
      /*@
       * @brief Checks to see if a faction has a static standing with the player.
       *
       * @usage b = f:static()
       *
       *    @luatparam Faction f Faction to check if has a static standing to the
       * player.
       *    @luatreturn boolean true if the faction is static to the player.
       * @luafunc static
       */
      methods.add_method("static", |_, this, ()| -> mlua::Result<bool> {
         Ok(this.with(|fct| fct.fixed())?)
      });
      /*@
       * @brief Gets the default AI of the faction.
       *
       *    @luatparam Faction f Faction to check the default AI of.
       *    @luatreturn string The default AI of the faction.
       * @luafunc default_ai
       */
      methods.add_method("default_ai", |_, this, ()| -> mlua::Result<String> {
         Ok(this.with(|fct| fct.data.ai.clone())?)
      });
      /*@
       * @brief Gets the overridden reputation value of a faction.
       *
       *    @luatparam Faction f Faction to get whether or not the reputation is
       * overridden and the value.
       *    @luatreturn number|nil The override reputation value or nil if not
       * overridden.
       * @luafunc reputationOverride
       */
      methods.add_method(
         "reputationOverride",
         |_, this, ()| -> mlua::Result<Option<f32>> { Ok(this.with(|fct| fct.r#override())?) },
      );
      /*@
       * @brief Sets the overridden reputation value of a faction.
       *
       *    @luatparam Faction f Faction to enable/disable reputation override of.
       *    @luatparam number|nil value Sets the faction reputation override to the value if
       * a number, or disables it if nil.
       * @luafunc setReputationOverride
       */
      methods.add_method(
         "setReputationOverride",
         |_, this, value: Option<f32>| -> mlua::Result<()> {
            Ok(this.with(|fct| fct.set_override(value))?)
         },
      );
      /*@
       * @brief Gets the tags a faction has.
       *
       * @usage for k,v in ipairs(f:tags()) do ... end
       * @usage if f:tags().likes_cheese then ... end
       * @usage if f:tags("generic") then ... end
       *
       *    @luatparam[opt=nil] string tag Tag to check if exists.
       *    @luatreturn table|boolean Table of tags where the name is the key and true
       * is the value or a boolean value if a string is passed as the second parameter
       * indicating whether or not the specified tag exists.
       * @luafunc tags
       */
      methods.add_method("tags", |lua, this, ()| -> mlua::Result<mlua::Table> {
         this.with(|fct| lua.create_table_from(fct.data.tags.iter().map(|s| (s.clone(), true))))?
      });
      /*@
       * @brief Adds a faction dynamically. Note that if the faction already exists as
       * a dynamic faction, the existing one is returned.
       *
       * @note Defaults to known.
       *
       *    @luatparam Faction|nil base Faction to base it off of or nil for new
       * faction.
       *    @luatparam string name Name to give the faction.
       *    @luatparam[opt] string display Display name to give the faction.
       *    @luatparam[opt] table params Table of parameters. Options include `ai`
       * (string) to set the AI to use, `clear_allies` (boolean) to clear all allies,
       * `clear_enemies` (boolean) to clear all enemies, `player` (number) to set the
       * default player standing, `colour` (string|colour) which represents the
       * factional colours.
       *    @luatreturn The newly created faction.
       * @luafunc dynAdd
       */
      methods.add_function(
         "dynAdd",
         |lua,
          (base, name, display, params): (
            Option<FactionRef>,
            String,
            Option<String>,
            Option<mlua::Table>,
         )|
          -> mlua::Result<Self> {
            let mut data = match FACTIONS.try_write() {
               Ok(d) => d,
               Err(e) => {
                  return Err(mlua::Error::RuntimeError(format!(
                     "unable to lock to add dynamic faction: {e}"
                  )));
               }
            };
            let params = params.unwrap_or_else(|| lua.create_table().unwrap());
            let (mut fd, api) = if let Some(reference) = base {
               let fct = &data.get(reference).context("faction not found")?;
               let base = &fct.data;
               let ai = params
                  .get::<Option<String>>("ai")?
                  .unwrap_or(base.ai.clone());
               let clear_allies: bool = params.get("clear_allies")?;
               let clear_enemies: bool = params.get("clear_enemies")?;
               let player_def = params
                  .get::<Option<f32>>("player")?
                  .unwrap_or(base.player_def);
               let colour = params
                  .get::<Option<Colour>>("colour")?
                  .unwrap_or(base.colour);
               let allies = if clear_allies {
                  Vec::new()
               } else {
                  base.allies.clone()
               };
               let enemies = if clear_enemies {
                  Vec::new()
               } else {
                  base.enemies.clone()
               };
               (
                  FactionData {
                     id: FactionRef::null(),
                     name,
                     displayname: display,
                     ai,
                     logo: base.logo.as_ref().map(|l| l.try_clone()).transpose()?,
                     colour,
                     player_def,
                     allies,
                     enemies,
                     // TODO more stuff
                     tags: base.tags.clone(),
                     f_dynamic: true,
                     f_static: true,
                     ..Default::default()
                  },
                  fct.api.clone(),
               )
            } else {
               let ai: String = params.get("ai").unwrap_or(String::new());
               (
                  FactionData {
                     name,
                     displayname: display,
                     ai,
                     f_dynamic: true,
                     f_static: true,
                     ..Default::default()
                  },
                  None,
               )
            };
            let id = data.insert_with_key(|k| {
               fd.id = k;
               Faction {
                  api,
                  standing: RwLock::new(Standing {
                     player: fd.player_def,
                     p_override: None,
                     f_known: fd.f_known,
                     f_invisible: fd.f_invisible,
                  }),
                  c: FactionC::new(&fd),
                  data: fd,
               }
            });

            GRID.write().unwrap().recompute(&data)?;
            Ok(id)
         },
      );
      /*@
       * @brief Adds or removes allies to a faction. Only works with dynamic factions.
       *
       *    @luatparam Faction fac Faction to add ally to.
       *    @luatparam Faction ally Faction to add as an ally.
       *    @luatparam[opt=false] boolean remove Whether or not to remove the ally
       * from the faction instead of adding it.
       * @luafunc dynAlly
       */
      methods.add_method_mut(
         "dynAlly",
         |_, this, (ally, remove): (FactionRef, Option<bool>)| -> mlua::Result<()> {
            let remove = remove.unwrap_or(false);
            let _ = this.with_mut(|fct| {
               if !fct.data.f_dynamic {
                  return Err(mlua::Error::RuntimeError(
                     "Can only add allies to dynamic factions".to_string(),
                  ));
               }
               if remove {
                  fct.remove_ally(ally);
               } else {
                  fct.add_ally(ally);
               }
               Ok(())
            })?;
            let data = match FACTIONS.try_write() {
               Ok(d) => d,
               Err(e) => {
                  return Err(mlua::Error::RuntimeError(format!(
                     "unable to modify dynamic faction: {e}"
                  )));
               }
            };
            GRID.write().unwrap().recompute(&data)?;
            Ok(())
         },
      );
      /*@
       * @brief Adds or removes enemies to a faction. Only works with dynamic
       * factions.
       *
       *    @luatparam Faction fac Faction to add enemy to.
       *    @luatparam Faction enemy Faction to add as an enemy.
       *    @luatparam[opt=false] boolean remove Whether or not to remove the enemy
       * from the faction instead of adding it.
       * @luafunc dynEnemy
       */
      methods.add_method_mut(
         "dynEnemy",
         |_, this, (enemy, remove): (FactionRef, Option<bool>)| -> mlua::Result<()> {
            let remove = remove.unwrap_or(false);
            let _ = this.with_mut(|fct| {
               if !fct.data.f_dynamic {
                  return Err(mlua::Error::RuntimeError(
                     "Can only add allies to dynamic factions".to_string(),
                  ));
               }
               if remove {
                  fct.remove_enemy(enemy);
               } else {
                  fct.add_enemy(enemy);
               }
               Ok(())
            })?;
            let data = match FACTIONS.try_write() {
               Ok(d) => d,
               Err(e) => {
                  return Err(mlua::Error::RuntimeError(format!(
                     "unable to modify dynamic faction: {e}"
                  )));
               }
            };
            GRID.write().unwrap().recompute(&data)?;
            Ok(())
         },
      );
   }
}

use mlua::{Value, ffi};
use std::ffi::c_void;
pub fn open_faction(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData> {
   let proxy = lua.create_proxy::<FactionRef>()?;

   if let mlua::Value::Nil = lua.named_registry_value("push_faction")? {
      let push_faction = lua.create_function(|lua, fct: i64| {
         let fct = FactionRef::from_ffi(fct);
         lua.create_userdata(fct)
      })?;
      lua.set_named_registry_value("push_faction", push_faction)?;

      let get_faction = lua.create_function(|_, mut ud: mlua::UserDataRefMut<FactionRef>| {
         let fct: *mut FactionRef = &mut *ud;
         Ok(Value::LightUserData(mlua::LightUserData(
            fct as *mut c_void,
         )))
      })?;
      lua.set_named_registry_value("get_faction", get_faction)?;
   }

   Ok(proxy)
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C-unwind" fn luaL_checkfaction(
   L: *mut mlua::lua_State,
   idx: c_int,
) -> naevc::FactionRef {
   unsafe {
      let fct = lua_tofaction(L, idx);
      if fct == FactionRef::null().as_ffi() {
         ffi::luaL_typerror(L, idx, c"faction".as_ptr() as *const c_char);
      }
      fct
   }
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C-unwind" fn luaL_validfaction(
   L: *mut mlua::lua_State,
   idx: c_int,
) -> naevc::FactionRef {
   unsafe {
      if ffi::lua_isstring(L, idx) != 0 {
         let ptr = ffi::lua_tostring(L, idx);
         let s = CStr::from_ptr(ptr);
         return match FactionRef::new(s.to_str().unwrap()) {
            Some(f) => f.as_ffi(),
            None => {
               warn!("faction not found");
               FactionRef::null().as_ffi()
            }
         };
      }
      let fct = lua_tofaction(L, idx);
      if fct == FactionRef::null().as_ffi() {
         ffi::luaL_typerror(L, idx, c"faction".as_ptr() as *const c_char);
      }
      fct
   }
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C-unwind" fn lua_isfaction(L: *mut mlua::lua_State, idx: c_int) -> c_int {
   (lua_tofaction(L, idx) != FactionRef::null().as_ffi()) as c_int
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C-unwind" fn lua_pushfaction(L: *mut mlua::lua_State, fct: naevc::FactionRef) {
   unsafe {
      ffi::lua_getfield(L, ffi::LUA_REGISTRYINDEX, c"push_faction".as_ptr());
      ffi::lua_pushinteger(L, fct);
      ffi::lua_call(L, 1, 1);
   }
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C-unwind" fn lua_tofaction(L: *mut mlua::lua_State, idx: c_int) -> naevc::FactionRef {
   unsafe {
      let idx = ffi::lua_absindex(L, idx);
      ffi::lua_getfield(L, ffi::LUA_REGISTRYINDEX, c"get_faction".as_ptr());
      ffi::lua_pushvalue(L, idx);
      let fct = match ffi::lua_pcall(L, 1, 1, 0) {
         ffi::LUA_OK => {
            let ptr = ffi::lua_touserdata(L, -1) as *mut FactionRef;
            if ptr.is_null() {
               FactionRef::null().as_ffi()
            } else {
               (*ptr).as_ffi()
            }
         }
         _ => FactionRef::null().as_ffi(),
      };
      ffi::lua_pop(L, 1);
      fct
   }
}

// Here be C API
use std::os::raw::{c_char, c_double, c_int};

#[unsafe(no_mangle)]
pub extern "C" fn faction_null() -> i64 {
   FactionRef::null().as_ffi()
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_isFaction(f: i64) -> c_int {
   match FACTIONS.read().unwrap().get(FactionRef::from_ffi(f)) {
      Some(_) => 1,
      None => 0,
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_exists(name: *const c_char) -> i64 {
   let ptr = unsafe { CStr::from_ptr(name) };
   let name = ptr.to_str().unwrap();
   FactionRef::new(name).unwrap_or(FactionRef::null()).as_ffi()
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_get(name: *const c_char) -> i64 {
   let ptr = unsafe { CStr::from_ptr(name) };
   let name = ptr.to_str().unwrap();
   match FactionRef::new(name) {
      Some(f) => f.as_ffi(),
      None => {
         warnx!(gettext("Faction '{}' not found in stack."), name);
         FactionRef::null().as_ffi()
      }
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_getAll() -> *mut i64 {
   let mut fcts: Vec<i64> = vec![];
   for (id, _) in FACTIONS.read().unwrap().iter() {
      fcts.push(id.as_ffi());
   }
   Array::new(fcts).into_ptr() as *mut i64
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_getAllVisible() -> *mut i64 {
   let mut fcts: Vec<i64> = vec![];
   for (id, fct) in FACTIONS.read().unwrap().iter() {
      if !fct.data.f_invisible {
         fcts.push(id.as_ffi());
      }
   }
   Array::new(fcts).into_ptr() as *mut i64
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_getKnown() -> *mut i64 {
   let mut fcts: Vec<i64> = vec![];
   for (id, val) in FACTIONS.read().unwrap().iter() {
      if !val.data.f_invisible && val.standing.read().unwrap().f_known {
         fcts.push(id.as_ffi());
      }
   }
   Array::new(fcts).into_ptr() as *mut i64
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_clearKnown() {
   for (_, val) in FACTIONS.read().unwrap().iter() {
      val.standing.write().unwrap().f_known = val.data.f_known;
   }
}

/// Helper function for the C-side
fn faction_c_with<F, R>(id: i64, f: F) -> Result<R>
where
   F: Fn(&Faction) -> R,
{
   let factions = FACTIONS.read().unwrap();
   match factions.get(FactionRef::from_ffi(id)) {
      Some(fct) => Ok(f(fct)),
      None => anyhow::bail!("faction not found"),
   }
}
fn faction_c_with_mut<F, R>(id: i64, f: F) -> Result<R>
where
   F: Fn(&mut Faction) -> R,
{
   let mut factions = FACTIONS.write().unwrap();
   match factions.get_mut(FactionRef::from_ffi(id)) {
      Some(fct) => {
         if fct.dynamic() {
            Ok(f(fct))
         } else {
            anyhow::bail!("trying to modify a non-dynamic faction!")
         }
      }
      None => anyhow::bail!("faction not found"),
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_isStatic(id: i64) -> i64 {
   faction_c_with(id, |fct| match fct.fixed() {
      true => 1,
      false => 0,
   })
   .unwrap_or_else(|err| {
      warn_err!(err);
      0
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_isInvisible(id: i64) -> i64 {
   faction_c_with(id, |fct| match fct.invisible() {
      true => 1,
      false => 0,
   })
   .unwrap_or_else(|err| {
      warn_err!(err);
      0
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_setInvisible(id: i64, state: i64) -> c_int {
   faction_c_with(id, |fct| {
      fct.set_invisible(!matches!(state, 0));
      0
   })
   .unwrap_or_else(|err| {
      warn_err!(err);
      -1
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_isKnown(id: i64) -> i64 {
   faction_c_with(id, |fct| match fct.known() {
      true => 1,
      false => 0,
   })
   .unwrap_or_else(|err| {
      warn_err!(err);
      0
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_isDynamic(id: i64) -> i64 {
   faction_c_with(id, |fct| match fct.dynamic() {
      true => 1,
      false => 0,
   })
   .unwrap_or_else(|err| {
      warn_err!(err);
      0
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_name(id: i64) -> *const c_char {
   faction_c_with(id, |fct| {
      // Not translated on purpose
      fct.c.cname.as_ptr()
   })
   .unwrap_or_else(|err| {
      warn_err!(err);
      std::ptr::null()
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_shortname(id: i64) -> *const c_char {
   faction_c_with(id, |fct| {
      let ptr = match &fct.c.cdisplayname {
         Some(name) => name.as_ptr(),
         None => fct.c.cname.as_ptr(),
      };
      unsafe { naevc::gettext_rust(ptr) }
   })
   .unwrap_or_else(|err| {
      warn_err!(err);
      std::ptr::null()
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_longname(id: i64) -> *const c_char {
   faction_c_with(id, |fct| {
      let ptr = match &fct.c.clongname {
         Some(name) => name.as_ptr(),
         None => match &fct.c.cdisplayname {
            Some(name) => name.as_ptr(),
            None => fct.c.cname.as_ptr(),
         },
      };
      unsafe { naevc::gettext_rust(ptr) }
   })
   .unwrap_or_else(|err| {
      warn_err!(err);
      std::ptr::null()
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_mapname(id: i64) -> *const c_char {
   faction_c_with(id, |fct| {
      let ptr = match &fct.c.cmapname {
         Some(name) => name.as_ptr(),
         None => match &fct.c.cdisplayname {
            Some(name) => name.as_ptr(),
            None => fct.c.cname.as_ptr(),
         },
      };
      unsafe { naevc::gettext_rust(ptr) }
   })
   .unwrap_or_else(|err| {
      warn_err!(err);
      std::ptr::null()
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_description(id: i64) -> *const c_char {
   faction_c_with(id, |fct| {
      let ptr = fct.c.cdescription.as_ptr();
      unsafe { naevc::gettext_rust(ptr) }
   })
   .unwrap_or_else(|err| {
      warn_err!(err);
      std::ptr::null()
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_default_ai(id: i64) -> *const c_char {
   faction_c_with(id, |fct| {
      if fct.data.ai.is_empty() {
         std::ptr::null()
      } else {
         fct.c.cai.as_ptr()
      }
   })
   .unwrap_or_else(|err| {
      warn_err!(err);
      std::ptr::null()
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_tags(id: i64) -> *mut *const c_char {
   faction_c_with(id, |fct| fct.c.ctags.as_ptr()).unwrap_or_else(|err| {
      warn_err!(err);
      std::ptr::null_mut()
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_lane_length_per_presence(id: i64) -> c_double {
   faction_c_with(id, |fct| fct.data.lane_length_per_presence as c_double).unwrap_or_else(|err| {
      warn_err!(err);
      0.0
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_lane_base_cost(id: i64) -> c_double {
   faction_c_with(id, |fct| fct.data.lane_base_cost as c_double).unwrap_or_else(|err| {
      warn_err!(err);
      0.0
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_logo(id: i64) -> *const texture::Texture {
   faction_c_with(id, |fct| match &fct.data.logo {
      Some(logo) => logo as *const texture::Texture,
      None => std::ptr::null(),
   })
   .unwrap_or_else(|err| {
      warn_err!(err);
      std::ptr::null()
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_colour(id: i64) -> *const naevc::glColour {
   faction_c_with(id, |fct| {
      &fct.data.colour as *const Colour as *const naevc::glColour
   })
   .unwrap_or_else(|err| {
      warn_err!(err);
      std::ptr::null()
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_setKnown(id: i64, state: i64) -> c_int {
   faction_c_with(id, |fct| {
      fct.set_known(!matches!(state, 0));
      0
   })
   .unwrap_or_else(|err| {
      warn_err!(err);
      -1
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_reputation(id: i64) -> c_double {
   faction_c_with(id, |fct| fct.player()).unwrap_or_else(|err| {
      warn_err!(err);
      0.0
   }) as c_double
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_setReputation(id: i64, value: c_double) {
   faction_c_with(id, |fct| fct.set_player(value as f32)).unwrap_or_else(|err| {
      warn_err!(err);
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_reputationOverride(id: i64, set: *mut c_int) -> c_double {
   faction_c_with(id, |fct| match fct.r#override() {
      Some(v) => {
         unsafe {
            *set = 1;
         }
         v
      }
      None => {
         unsafe {
            *set = 0;
         }
         0.0
      }
   })
   .unwrap_or_else(|_err| {
      //warn_err!(err);
      unsafe {
         *set = 0;
      }
      0.0
   }) as c_double
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_setReputationOverride(id: i64, set: c_int, value: c_double) {
   faction_c_with_mut(id, |fct| {
      fct.set_override(if set == 0 { None } else { Some(value as f32) });
   })
   .unwrap_or_else(|err| {
      warn_err!(err);
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_isPlayerFriend(id: i64) -> c_int {
   FactionRef::from_ffi(id).player_ally(None) as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_isPlayerEnemy(id: i64) -> c_int {
   FactionRef::from_ffi(id).player_enemy(None) as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_isPlayerFriendSystem(id: i64, sys: *const naevc::StarSystem) -> c_int {
   let sys = if sys.is_null() {
      None
   } else {
      Some(unsafe { &*sys })
   };
   FactionRef::from_ffi(id).player_ally(sys) as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_isPlayerEnemySystem(id: i64, sys: *const naevc::StarSystem) -> c_int {
   let sys = if sys.is_null() {
      None
   } else {
      Some(unsafe { &*sys })
   };
   FactionRef::from_ffi(id).player_enemy(sys) as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn areEnemies(a: i64, b: i64) -> c_int {
   areEnemiesSystem(a, b, std::ptr::null())
}

#[unsafe(no_mangle)]
pub extern "C" fn areNeutral(a: i64, b: i64) -> c_int {
   let a = FactionRef::from_ffi(a);
   let b = FactionRef::from_ffi(b);
   a.are_neutrals(&b) as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn areAllies(a: i64, b: i64) -> c_int {
   areAlliesSystem(a, b, std::ptr::null())
}

#[unsafe(no_mangle)]
pub extern "C" fn areEnemiesSystem(a: i64, b: i64, sys: *const naevc::StarSystem) -> c_int {
   let sys = if sys.is_null() {
      None
   } else {
      Some(unsafe { &*sys })
   };
   let a = FactionRef::from_ffi(a);
   let b = FactionRef::from_ffi(b);
   a.are_enemies(&b, sys) as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn areAlliesSystem(a: i64, b: i64, sys: *const naevc::StarSystem) -> c_int {
   let sys = if sys.is_null() {
      None
   } else {
      Some(unsafe { &*sys })
   };
   let a = FactionRef::from_ffi(a);
   let b = FactionRef::from_ffi(b);
   a.are_allies(&b, sys) as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_getStandingText(id: i64) -> *const c_char {
   faction_getStandingTextAtValue(id, faction_reputation(id))
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_getStandingTextAtValue(id: i64, value: c_double) -> *const c_char {
   static STANDING: Mutex<Option<CString>> = Mutex::new(None);
   let rank = match faction_c_with(id, |fct| fct.text_rank(Some(value as f32))).flatten() {
      Ok(r) => r,
      Err(_e) => {
         //warn_err!(e);
         return std::ptr::null();
      }
   };
   let mut txt = STANDING.lock().unwrap();
   *txt = Some(CString::new(rank).unwrap());
   txt.as_ref().unwrap().as_ptr()
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_reputationMax(id: i64) -> c_double {
   faction_c_with(id, |fct| fct.reputation_max())
      .flatten()
      .unwrap_or_else(|_err| {
         //warn_err!(err);
         0.0
      })
      .into()
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_usesHiddenJumps(id: i64) -> c_int {
   (faction_c_with(id, |fct| fct.data.f_useshiddenjumps).unwrap_or(false)) as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_hit(
   id: i64,
   sys: *const naevc::StarSystem,
   value: c_double,
   source: *const c_char,
   secondary: c_int,
) -> c_double {
   let sys = crate::system::to_lua(&NLUA.lua, sys).unwrap();
   let src = unsafe { CStr::from_ptr(source) }.to_str().unwrap();
   FactionRef::from_ffi(id)
      .hit(value as f32, &sys, src, secondary != 0)
      .unwrap_or_else(|e| {
         warn_err!(e);
         0.0
      })
      .into()
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_addEnemy(id: i64, other: i64) {
   faction_c_with_mut(id, |fct| fct.add_enemy(FactionRef::from_ffi(other))).unwrap_or_else(|err| {
      warn_err!(err);
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_rmEnemy(id: i64, other: i64) {
   faction_c_with_mut(id, |fct| fct.remove_enemy(FactionRef::from_ffi(other))).unwrap_or_else(
      |err| {
         warn_err!(err);
      },
   )
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_addAlly(id: i64, other: i64) {
   faction_c_with_mut(id, |fct| fct.add_ally(FactionRef::from_ffi(other))).unwrap_or_else(|err| {
      warn_err!(err);
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_rmAlly(id: i64, other: i64) {
   faction_c_with_mut(id, |fct| fct.remove_ally(FactionRef::from_ffi(other))).unwrap_or_else(
      |err| {
         warn_err!(err);
      },
   )
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_addNeutral(id: i64, other: i64) {
   faction_c_with_mut(id, |fct| fct.add_neutral(FactionRef::from_ffi(other))).unwrap_or_else(
      |err| {
         warn_err!(err);
      },
   )
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_rmNeutral(id: i64, other: i64) {
   faction_c_with_mut(id, |fct| fct.remove_neutral(FactionRef::from_ffi(other))).unwrap_or_else(
      |err| {
         warn_err!(err);
      },
   )
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_getEquipper(id: i64) -> *const naevc::nlua_env {
   faction_c_with(id, |fct| {
      if let Some(api) = &fct.api
         && let Some(env) = &api.equip_env
      {
         Arc::as_ptr(env) as *const naevc::nlua_env
      } else {
         std::ptr::null()
      }
   })
   .unwrap_or_else(|err| {
      warn_err!(err);
      std::ptr::null()
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_getScheduler(id: i64) -> *const naevc::nlua_env {
   faction_c_with(id, |fct| {
      if let Some(api) = &fct.api
         && let Some(env) = &api.sched_env
      {
         Arc::as_ptr(env) as *const naevc::nlua_env
      } else {
         std::ptr::null()
      }
   })
   .unwrap_or_else(|err| {
      warn_err!(err);
      std::ptr::null()
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn factions_clearDynamic() {
   let mut data = FACTIONS.write().unwrap();
   data.retain(|_id, fct| !fct.data.f_dynamic);
   let _ = GRID.write().unwrap().recompute(&data);
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_updateSingle(id: i64) {
   let mut v = (0.0, 0.0);
   for sys in crate::system::get() {
      let p = sys.presence();
      v = p.iter().fold(v, |val, sp| {
         if sp.value > 0.0 && sp.faction == id {
            (val.0 + sp.local as f32, val.1 + 1.0)
         } else {
            val
         }
      });
   }

   // Propagate and update
   if let Err(e) = FactionRef::from_ffi(id)
      .with(|f| f.standing.write().unwrap().player = if v.1 > 0.0 { v.0 / v.1 } else { 0.0 })
   {
      warn_err!(e);
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_updateGlobal() {
   for (id, _fct) in FACTIONS.read().unwrap().iter() {
      faction_updateSingle(id.as_ffi());
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn factions_resetLocal() {
   for sys in crate::system::get_mut() {
      for sp in sys.presence_mut() {
         sp.local = faction_reputation(sp.faction);
      }
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn factions_reset() {
   factions_clearDynamic();
   // Reset global standing.
   for (_id, fct) in FACTIONS.read().unwrap().iter() {
      let mut standing = fct.standing.write().unwrap();
      standing.p_override = None;
      standing.player = fct.data.player_def;
      // TODO more flags?
      standing.f_known = fct.data.f_known;
   }
   factions_resetLocal();
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_player() -> i64 {
   PLAYER.get().unwrap().as_ffi()
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_reputationColourChar(id: i64) -> c_char {
   faction_reputationColourCharSystem(id, std::ptr::null())
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_reputationColourCharSystem(
   id: i64,
   sys: *const naevc::StarSystem,
) -> c_char {
   let sys = if sys.is_null() {
      None
   } else {
      Some(unsafe { &*sys })
   };
   let f = FactionRef::from_ffi(id);
   let player = PLAYER.get().unwrap();
   (if FACTIONS.read().unwrap().get(f).is_none() {
      'I'
   } else if f.are_enemies(player, sys) {
      'H'
   } else if f.are_allies(player, sys) {
      'F'
   } else {
      'N'
   }) as c_char
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_getEnemies(id: i64) -> *const i64 {
   static ENEMY_ARRAY: LazyLock<Mutex<Array<i64>>> = LazyLock::new(|| Mutex::new(Array::default()));
   let data = if FactionRef::from_ffi(id) == *PLAYER.get().unwrap() {
      FACTIONS
         .read()
         .unwrap()
         .iter()
         .filter_map(|(id, fct)| {
            if fct.player() < 0.0 {
               Some(id.as_ffi())
            } else {
               None
            }
         })
         .collect::<Vec<_>>()
   } else {
      faction_c_with(id, |fct| {
         fct.data
            .enemies
            .iter()
            .map(|f| f.as_ffi())
            .collect::<Vec<_>>()
      })
      .unwrap_or_else(|err| {
         warn_err!(err);
         Vec::new()
      })
   };
   let mut array = ENEMY_ARRAY.lock().unwrap();
   *array = Array::new(data);
   array.as_ptr() as *const i64
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_getAllies(id: i64) -> *const i64 {
   static ALLY_ARRAY: LazyLock<Mutex<Array<i64>>> = LazyLock::new(|| Mutex::new(Array::default()));
   let data = if FactionRef::from_ffi(id) == *PLAYER.get().unwrap() {
      FACTIONS
         .read()
         .unwrap()
         .iter()
         .filter_map(|(id, fct)| {
            let friendly_at = match &fct.api {
               Some(api) => api.friendly_at,
               None => f32::INFINITY,
            };
            if fct.player() > friendly_at {
               Some(id.as_ffi())
            } else {
               None
            }
         })
         .collect::<Vec<_>>()
   } else {
      faction_c_with(id, |fct| {
         fct.data
            .allies
            .iter()
            .map(|f| f.as_ffi())
            .collect::<Vec<_>>()
      })
      .unwrap_or_else(|err| {
         warn_err!(err);
         Vec::new()
      })
   };
   let mut array = ALLY_ARRAY.lock().unwrap();
   *array = Array::new(data);
   array.as_ptr() as *const i64
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_getGroup(which: c_int, sys: *const naevc::StarSystem) -> *mut i64 {
   let sys = if sys.is_null() {
      None
   } else {
      Some(unsafe { &*sys })
   };
   let fcts: Vec<i64> = if which == 0 {
      // all factions
      FACTIONS
         .read()
         .unwrap()
         .keys()
         .map(|id| id.as_ffi())
         .collect()
   } else if which == 1 {
      // friendly
      FACTIONS
         .read()
         .unwrap()
         .keys()
         .filter_map(|id| {
            if id.player_ally(sys) {
               Some(id.as_ffi())
            } else {
               None
            }
         })
         .collect()
   } else if which == 2 {
      // neutral
      FACTIONS
         .read()
         .unwrap()
         .keys()
         .filter_map(|id| {
            if !id.player_enemy(sys) && !id.player_ally(sys) {
               Some(id.as_ffi())
            } else {
               None
            }
         })
         .collect()
   } else if which == 3 {
      // enemy
      FACTIONS
         .read()
         .unwrap()
         .keys()
         .filter_map(|id| {
            if id.player_enemy(sys) {
               Some(id.as_ffi())
            } else {
               None
            }
         })
         .collect()
   } else {
      return std::ptr::null_mut();
   };
   Array::new(fcts).into_ptr() as *mut i64
}

#[unsafe(no_mangle)]
pub extern "C" fn faction_generators(id: i64) -> *const naevc::FactionGenerator {
   let generators = faction_c_with(id, |fct| {
      fct.data
         .generators
         .iter()
         .map(|g| naevc::FactionGenerator {
            id: g.id.as_ffi(),
            weight: g.weight.into(),
         })
         .collect::<Vec<_>>()
   })
   .unwrap_or_else(|e| {
      warn_err!(e);
      vec![]
   });

   static GENERATOR_ARRAY: LazyLock<Mutex<Array<naevc::FactionGenerator>>> =
      LazyLock::new(|| Mutex::new(Array::default()));
   let mut array = GENERATOR_ARRAY.lock().unwrap();
   *array = Array::new(generators);
   array.as_ptr() as *const naevc::FactionGenerator
}

/*
#[test]
fn test_reputation_raw () {
   use approx::{abs_diff_ne, abs_diff_eq};
   for rep in -100..=100 {
      let rep = rep as f32;
      let raw = reputation_to_raw( rep );
      let _ = abs_diff_ne!( rep, raw );
      let _ = abs_diff_eq!( rep, raw_to_reputation( raw ) );
      let _ = abs_diff_eq!( -raw, reputation_to_raw( -rep ) );
      let _ = abs_diff_eq!( -rep, raw_to_reputation( -raw ) );
      let _ = abs_diff_eq!( rep, -raw_to_reputation( -raw ) );
   }
}
*/
