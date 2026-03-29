#![allow(dead_code, unused)]
use crate::faction::FactionRef;
use renderer::texture::{Texture, TextureBuilder};
use slotmap::SlotMap;
use std::sync::atomic::AtomicI64;
use std::sync::{LazyLock, RwLock};

struct PriceRef {
   base: CommodityRef,
   modifier: f64,
}

struct EconomyModifiers {
   period: f64,
   population_modifier: f64,
}

struct Commodity {
   name: String,
   display: String,
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
}

slotmap::new_key_type! {
pub struct CommodityRef;
}

static COMMODITIES: LazyLock<RwLock<SlotMap<CommodityRef, Commodity>>> =
   LazyLock::new(|| RwLock::new(SlotMap::with_key()));
