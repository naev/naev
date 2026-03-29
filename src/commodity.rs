#![allow(dead_code, unused)]
use crate::array::{Array, ArrayCString};
use crate::faction::FactionRef;
use anyhow::Context as AnyhowContext;
use anyhow::Result;
use gettext::gettext;
use helpers::ReferenceC;
use naev_core::{nxml, nxml_err_attr_missing, nxml_warn_node_unknown};
use nlog::{warn, warn_err, warnx};
use renderer::texture::TextureDeserializer;
use renderer::{Context, ContextWrapper, texture};
use slotmap::{Key, KeyData, SecondaryMap, SlotMap};
use std::collections::HashMap;
use std::ffi::{CString, OsStr};
use std::fmt::Debug;
use std::path::{Path, PathBuf};
use std::sync::atomic::AtomicI64;
use std::sync::{LazyLock, RwLock};
use tracing::instrument;

#[derive(Default, Debug)]
struct PriceRef {
   base: CommodityRef,
   modifier: f64,
}

#[derive(Default, Debug)]
struct EconomyModifiers {
   period: f64,
   population_modifier: f64,
   spob_modifier: HashMap<String, f32>,
   faction_modifier: HashMap<FactionRef, f32>,
}

#[derive(Default, Debug)]
struct CommodityC {
   name: CString,
   display: Option<CString>,
   description: CString,
   illegal_to: Array<FactionRef>,
   ctags: ArrayCString,
}
impl CommodityC {
   fn new(com: &Commodity) -> Result<Self> {
      Ok(CommodityC {
         name: CString::new(&*com.name)?,
         display: com.display.clone().map(|d| CString::new(&*d)).transpose()?,
         description: CString::new(&*com.description)?,
         illegal_to: Array::new(&com.illegal_to)?,
         ctags: ArrayCString::new(&com.tags)?,
      })
   }
}

#[derive(Default, Debug)]
struct CommodityLoad {
   price_ref: Option<String>,
   price_mod: Option<f64>,
}

#[derive(Default, Debug)]
struct Commodity {
   id: CommodityRef,
   name: String,
   display: Option<String>,
   description: String,

   temporary: bool,
   standard: bool,
   always_can_sell: bool,
   price_constant: bool,

   price: i64,
   price_ref: Option<PriceRef>,

   gfx_store: Option<texture::Texture>,
   gfx_space: Option<texture::Texture>,

   last_purchased_price: AtomicI64,

   economy_modifiers: EconomyModifiers,

   illegal_to: Vec<FactionRef>,
   tags: Vec<String>,

   // C stuff, TODO remove when possible
   c: CommodityC,
}
impl Commodity {
   #[instrument(skip(ctx))]
   fn new<P: AsRef<Path> + Debug>(
      ctx: &ContextWrapper,
      filename: P,
   ) -> Result<(Self, CommodityLoad)> {
      let mut com = Self::default();
      let mut load = CommodityLoad::default();

      let data = ndata::read(filename)?;
      let doc = roxmltree::Document::parse(std::str::from_utf8(&data)?)?;
      let root = doc.root_element();
      for node in root.children() {
         if !node.is_element() {
            continue;
         }
         match node.tag_name().name().to_lowercase().as_str() {
            "name" => com.name = nxml::node_string(node)?,
            "display" => com.display = Some(nxml::node_string(node)?),
            "description" => com.description = nxml::node_string(node)?,
            "price" => com.price = nxml::node_str(node)?.parse()?,
            "price_mod" => load.price_mod = Some(nxml::node_str(node)?.parse()?),
            "price_ref" => load.price_ref = Some(nxml::node_string(node)?),
            "gfx_space" => {
               let gfxname = nxml::node_texturepath(node, "gfx/commodity/space/")?;
               com.gfx_space = Some(
                  texture::TextureBuilder::new()
                     .path(&gfxname)
                     .build_wrap(ctx)?,
               );
            }
            "gfx_store" => {
               let gfxname = nxml::node_texturepath(node, "gfx/commodity/")?;
               com.gfx_store = Some(
                  texture::TextureBuilder::new()
                     .path(&gfxname)
                     .build_wrap(ctx)?,
               );
            }
            "standard" => com.standard = true,
            "always_can_sell" => com.always_can_sell = true,
            "price_constant" => com.price_constant = true,
            "illegalto" => {
               for node in node.children() {
                  if !node.is_element() {
                     continue;
                  }
                  match node.tag_name().name().to_lowercase().as_str() {
                     "faction" => {
                        let name = nxml::node_str(node)?;
                        com.illegal_to.push(
                           FactionRef::new(name)
                              .with_context(|| format!("finding faction '{name}'"))?,
                        );
                     }
                     tag => nxml_warn_node_unknown!("Commodity", &com.name, tag),
                  }
               }
            }
            "population_modifier" => {
               com.economy_modifiers.population_modifier = nxml::node_str(node)?.parse()?
            }
            "period" => com.economy_modifiers.period = nxml::node_str(node)?.parse()?,
            "spob_modifier" => {
               let name = String::from(match node.attribute("type") {
                  Some(n) => n,
                  None => {
                     return nxml_err_attr_missing!("Commodity/spob_modifier", "type");
                  }
               });
               let value: f32 = nxml::node_str(node)?.parse()?;
               com.economy_modifiers.spob_modifier.insert(name, value);
            }
            "faction_modifier" => {
               let name = match node.attribute("type") {
                  Some(n) => n,
                  None => {
                     return nxml_err_attr_missing!("Commodity/faction_modifier", "type");
                  }
               };
               let value: f32 = nxml::node_str(node)?.parse()?;
               let fct =
                  FactionRef::new(name).with_context(|| format!("finding faction '{name}'"))?;
               com.economy_modifiers.faction_modifier.insert(fct, value);
            }
            "tags" => {
               for node in node.children() {
                  if !node.is_element() {
                     continue;
                  }
                  match node.tag_name().name().to_lowercase().as_str() {
                     "tag" => com.tags.push(nxml::node_string(node)?),
                     tag => nxml_warn_node_unknown!("Commodity", &com.name, tag),
                  }
               }
            }
            // Case we missed everything
            tag => nxml_warn_node_unknown!("Commodity", &com.name, tag),
         }
      }
      Ok((com, load))
   }

   pub fn name(&self) -> &str {
      if let Some(display) = &self.display {
         display
      } else {
         &self.name
      }
   }
}

slotmap::new_key_type! {
pub struct CommodityRef;
}
impl CommodityRef {
   pub fn from_ffi(value: i64) -> Self {
      Self(KeyData::from_ffi(value as u64))
   }

   #[instrument]
   pub fn new(name: &str) -> Option<Self> {
      let commodities = COMMODITIES.read().unwrap();
      for (id, com) in commodities.iter() {
         if com.name == name {
            return Some(id);
         }
      }
      None
   }
}

static COMMODITIES: LazyLock<RwLock<SlotMap<CommodityRef, Commodity>>> =
   LazyLock::new(|| RwLock::new(SlotMap::with_key()));

#[instrument]
pub fn load() -> Result<()> {
   // Since we hardcode this C side, we have to make sure it is in-fact correct.
   // Not static, so we have to do it runtime at the moment.
   //assert_eq!(
   //   i64::from_ne_bytes( naevc::COMMODITY_NULL.to_ne_bytes() ),
   //   CommodityRef::null().as_ffi()
   //);

   let ctx = Context::get().as_safe_wrap();
   let base: PathBuf = "commodities/".into();
   let files: Vec<_> = ndata::read_dir(&base)?
      .into_iter()
      .filter(|filename| filename.extension() == Some(OsStr::new("xml")))
      .collect();
   let mut data = COMMODITIES.write().unwrap();
   let mut load = SecondaryMap::new();
   let mut commap: HashMap<String, CommodityRef> = HashMap::new();

   // First pass, load primary data
   for f in files.into_iter() {
      match Commodity::new(&ctx, &base.join(f)) {
         Ok((mut com, comload)) => {
            let id = data.insert_with_key(|k| {
               com.id = k;
               commap.insert(com.name.clone(), k);
               com
            });
            load.insert(id, comload);
         }
         Err(e) => {
            warn_err!(e);
         }
      }
   }

   fn update_commodity(
      com: &mut Commodity,
      comload: &CommodityLoad,
      commap: &HashMap<String, CommodityRef>,
   ) -> Result<()> {
      if let Some(price_ref) = &comload.price_ref
         && let Some(com_mod) = comload.price_mod
      {
         com.price_ref = Some(PriceRef {
            base: *commap.get(price_ref).with_context(|| {
               format!(
                  "commodity '{}' has missing price_ref '{price_ref}'",
                  &com.name
               )
            })?,
            modifier: com_mod,
         });
      } else if comload.price_ref.is_some() || comload.price_mod.is_some() {
         warn!(
            "Commodity '{}' has either price_mod or price_ref set",
            &com.name
         );
      }

      com.c = CommodityC::new(&com)?;

      Ok(())
   }

   // Second pass - set up references and C stuff
   for (id, com) in data.iter_mut() {
      let comload = load.get(id).unwrap();
      if let Err(e) = update_commodity(com, &comload, &commap) {
         warn_err!(e);
      }
   }

   Ok(())
}

use std::ffi::{CStr, c_char, c_double, c_int};

#[unsafe(no_mangle)]
pub extern "C" fn _commodity_getAll() -> *mut i64 {
   let mut coms: Vec<i64> = vec![];
   for (id, _) in COMMODITIES.read().unwrap().iter() {
      coms.push(id.as_ffi());
   }
   Array::new(&coms).unwrap().into_ptr() as *mut i64
}

#[unsafe(no_mangle)]
pub extern "C" fn _commodity_get(name: *const c_char) -> i64 {
   let ptr = unsafe { CStr::from_ptr(name) };
   let name = ptr.to_str().unwrap();
   match CommodityRef::new(name) {
      Some(c) => c.as_ffi(),
      None => {
         warnx!(gettext("Commodity '{}' not found in stack."), name);
         CommodityRef::null().as_ffi()
      }
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn _commodity_getW(name: *const c_char) -> i64 {
   let ptr = unsafe { CStr::from_ptr(name) };
   let name = ptr.to_str().unwrap();
   CommodityRef::new(name)
      .unwrap_or(CommodityRef::null())
      .as_ffi()
}

/// Helper function for the C-side
fn faction_c_call<F, R>(id: i64, f: F) -> Result<R>
where
   F: Fn(&Commodity) -> R,
{
   let commodities = COMMODITIES.read().unwrap();
   match commodities.get(CommodityRef::from_ffi(id)) {
      Some(com) => Ok(f(com)),
      None => anyhow::bail!("commodity not found"),
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn _commodity_name(com: i64) -> *const c_char {
   faction_c_call(com, |c| unsafe {
      naevc::gettext_rust(if let Some(display) = &c.c.display {
         display.as_ptr()
      } else {
         c.c.name.as_ptr()
      })
   })
   .unwrap_or_else(|e| {
      warn_err!(e);
      std::ptr::null()
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn _commodity_name_raw(com: i64) -> *const c_char {
   faction_c_call(com, |c| c.c.name.as_ptr()).unwrap_or_else(|e| {
      warn_err!(e);
      std::ptr::null()
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn _commodity_description(com: i64) -> *const c_char {
   faction_c_call(com, |c| c.c.description.as_ptr()).unwrap_or_else(|e| {
      warn_err!(e);
      std::ptr::null()
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn _commodity_gfxStore(com: i64) -> *const texture::Texture {
   faction_c_call(com, |c| match &c.gfx_store {
      Some(tex) => tex as *const texture::Texture,
      None => std::ptr::null(),
   })
   .unwrap_or_else(|e| {
      warn_err!(e);
      std::ptr::null()
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn _commodity_gfxSpace(com: i64) -> *const texture::Texture {
   faction_c_call(com, |c| match &c.gfx_space {
      Some(tex) => tex as *const texture::Texture,
      None => std::ptr::null(),
   })
   .unwrap_or_else(|e| {
      warn_err!(e);
      std::ptr::null()
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn _commodity_illegalTo(com: i64) -> *const FactionRef {
   faction_c_call(com, |c| c.c.illegal_to.as_ptr() as *const FactionRef).unwrap_or_else(|e| {
      warn_err!(e);
      std::ptr::null()
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn _commodity_price(com: i64) -> i64 {
   faction_c_call(com, |c| c.price).unwrap_or_else(|e| {
      warn_err!(e);
      0
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn _commodity_price_constant(com: i64) -> c_int {
   faction_c_call(com, |c| c.price_constant as c_int).unwrap_or_else(|e| {
      warn_err!(e);
      0
   })
}
