#![allow(dead_code, unused)]
use crate::array::{Array, ArrayCString};
use crate::faction::FactionRef;
use renderer::texture::{Texture, TextureBuilder};
use slotmap::SlotMap;
use std::collections::HashMap;
use std::ffi::CString;
use std::sync::atomic::AtomicI64;
use std::sync::{LazyLock, RwLock};

#[derive(Debug)]
struct PriceRef {
   base: CommodityRef,
   modifier: f64,
}

#[derive(Debug)]
struct EconomyModifiers {
   period: f64,
   population_modifier: f64,
   spob_modifier: HashMap<String, f32>,
   faction_modifier: HashMap<FactionRef, f32>,
}

#[derive(Debug)]
struct CommodityC {
   name: CString,
   display: Option<CString>,
   description: CString,
   illegal_to: Array<FactionRef>,
   ctags: ArrayCString,
}

#[derive(Debug)]
struct Commodity {
   name: String,
   display: Option<String>,
   description: String,

   temporary: bool,
   standard: bool,
   always_can_sell: bool,
   price_constant: bool,

   price: f64,
   price_ref: Option<PriceRef>,

   gfx_store: Texture,
   gfx_space: Option<Texture>,

   last_purchased_price: AtomicI64,

   economy_modifiers: EconomyModifiers,

   illegal_to: Vec<FactionRef>,
   tags: Vec<String>,

   // C stuff, TODO remove when possible
   c: CommodityC,
}
impl Commodity {
   pub fn name(&self) -> &str {
      if let Some(display) = &self.display {
         &display
      } else {
         &self.name
      }
   }
}

slotmap::new_key_type! {
pub struct CommodityRef;
}

static COMMODITIES: LazyLock<RwLock<SlotMap<CommodityRef, Commodity>>> =
   LazyLock::new(|| RwLock::new(SlotMap::with_key()));
