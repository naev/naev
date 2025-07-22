use rayon::prelude::*;
use renderer::texture::TextureBuilder;
use renderer::Context;
use std::ffi::{c_void, CStr};

struct OutfitWrapper(naevc::Outfit);
//unsafe impl Sync for OutfitWrapper {}
unsafe impl Send for OutfitWrapper {}

#[allow(dead_code)]
fn get() -> &'static [OutfitWrapper] {
    unsafe {
        let outfits = naevc::outfit_getAll_rust();
        let n = naevc::array_size_rust(outfits as *const c_void) as usize;
        std::slice::from_raw_parts(outfits as *const OutfitWrapper, n)
    }
}

fn get_mut() -> &'static mut [OutfitWrapper] {
    unsafe {
        let outfits = naevc::outfit_getAll_rust();
        let n = naevc::array_size_rust(outfits as *const c_void) as usize;
        std::slice::from_raw_parts_mut(outfits as *mut OutfitWrapper, n)
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn outfit_gfxStoreLoadNeeded() {
    let ctx = Context::get().as_safe_wrap();
    get_mut().par_iter_mut().for_each(|ptr| {
        let o = &mut ptr.0;
        if o.properties & naevc::OUTFIT_PROP_NEEDSGFX == 0 {
            return;
        }
        o.properties &= !naevc::OUTFIT_PROP_NEEDSGFX;
        if !o.gfx_store.is_null() || o.gfx_store_path.is_null() {
            return;
        }

        let gfx_path = unsafe { CStr::from_ptr(o.gfx_store_path).to_str().unwrap() };
        let path = {
            match gfx_path.chars().next() {
                Some('/') => String::from(gfx_path),
                _ => format!("gfx/outfit/store/{gfx_path}"),
            }
        };

        let tex = TextureBuilder::new().path(&path).build_wrap(&ctx).unwrap();
        o.gfx_store = tex.into_ptr() as *mut naevc::glTexture;
    });
}

/*
use crate::slots::SlotProperty;

pub enum SlotType {
   OUTFIT_SLOT_NULL,      /**< Invalid slot type. */
   OUTFIT_SLOT_NA,        /**< Slot type not applicable. */
   OUTFIT_SLOT_INTRINSIC, /**< Internal outfits that don't use slots. */
   OUTFIT_SLOT_STRUCTURE, /**< Low energy slot. */
   OUTFIT_SLOT_UTILITY,   /**< Medium energy slot. */
   OUTFIT_SLOT_WEAPON     /**< High energy slot. */
}

pub enum SlotSize {
   OUTFIT_SLOT_SIZE_NA,     /**< Not applicable slot size. */
   OUTFIT_SLOT_SIZE_LIGHT,  /**< Light slot size. */
   OUTFIT_SLOT_SIZE_MEDIUM, /**< Medium slot size. */
   OUTFIT_SLOT_SIZE_HEAVY   /**< Heavy slot size. */
}

pub struct Slot {
    pub property: Option<SlotProperty>,
    pub exclusive: bool,
    pub slottype: SlotType,
    pub size: SlotSize,
}

pub struct Outfit<T:?Sized> {
    name: String,
    typename: String,
    shortname: String,
    rarity: u8,
    filename: String,

    slot: Slot,
    slot_extra: Option<SlotProperty>,
    license: String,
    cond: String,
    condstr: String,
    mass: f64,
    cpu: u32,
    limit: String,
    //illegalto: Array<Faction>,
    illegaltoS: String,

    price: u64,
    desc_raw: String,
    summary_raw: String,
    desc_extra: String,
    priority: u8,

    gfx_store_path: String,
    gfx_store: Option<Texture>,
    gfx_overlays: Vector<Texture>,

    // Properties
    unique: bool,
    shoot_dry: bool,
    template: bool,
    weap_secondary: bool,
    weap_spin: bool,
    weap_blowup_armour: bool,
    weap_blowup_shield: bool,
    weap_friendlyfire: bool,
    weap_pointdefense: bool,
    weap_miss_ships: bool,
    weap_miss_asterids: bool,
    weap_miss_explode: bool,
    weap_onlyhittarget: bool,
    weap_collision_override: bool,
    weap_needsgfx: bool,
    weap_stealth_on: bool,
    weap_hidestats: bool,

    d: T,
}

/// Active outfits
trait ActiveOutfit {
}

/// Bolt weapon specifics
pub struct BoltWeapon {
}
impl ActiveOutfit for BoltWeapon {
}

impl Outfit<BoltWeapon> {
    // Bolt weapon specific
}

impl<T:ActiveOutfit + ?Sized> Outfit<T> {
    // Work with any that implements ActiveOutfit
}
*/
