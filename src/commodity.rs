#![allow(dead_code, unused)]
use crate::array::{Array, ArrayCString};
use crate::faction::FactionRef;
use anyhow::Context as AnyhowContext;
use anyhow::Result;
use gettext::gettext;
use helpers::ReferenceC;
use mlua::{
   BorrowedStr, Either, FromLua, Function, MetaMethod, UserData, UserDataMethods, UserDataRef,
};
use naev_core::{nxml, nxml_err_attr_missing, nxml_warn_node_unknown};
use nlog::{warn, warn_err, warnx};
use renderer::texture::TextureDeserializer;
use renderer::{Context, ContextWrapper, texture};
use slotmap::{Key, KeyData, SecondaryMap, SlotMap};
use std::collections::HashMap;
use std::ffi::{CString, OsStr};
use std::fmt::Debug;
use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicI64, Ordering};
use std::sync::{LazyLock, RwLock};
use tracing::instrument;

#[derive(Default, Debug)]
pub struct PriceRef {
   base: CommodityRef,
   modifier: f64,
}

#[derive(Default, Debug)]
pub struct EconomyModifiers {
   period: f64,
   population_modifier: f64,
   spob_modifier: HashMap<String, f32>,
   faction_modifier: HashMap<FactionRef, f32>,
}

#[derive(Default, Debug)]
pub struct CommodityC {
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
pub struct Commodity {
   pub id: CommodityRef,
   pub name: String,
   pub display: Option<String>,
   pub description: String,

   pub temporary: bool,
   pub standard: bool,
   pub always_can_sell: bool,
   pub price_constant: bool,

   pub price: i64,
   pub price_ref: Option<PriceRef>,

   pub gfx_store: Option<texture::Texture>,
   pub gfx_space: Option<texture::Texture>,

   pub last_purchased_price: AtomicI64,

   pub economy_modifiers: EconomyModifiers,

   pub illegal_to: Vec<FactionRef>,
   pub tags: Vec<String>,

   // C stuff, TODO remove when possible
   pub c: CommodityC,
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

   pub fn call<F, R>(&self, f: F) -> Result<R>
   where
      F: Fn(&Commodity) -> R,
   {
      let commodities = COMMODITIES.read().unwrap();
      match commodities.get(*self) {
         Some(com) => Ok(f(com)),
         None => anyhow::bail!("commodity not found"),
      }
   }

   pub fn call_mut<F, R>(&self, f: F) -> Result<R>
   where
      F: Fn(&mut Commodity) -> R,
   {
      let mut commodities = COMMODITIES.write().unwrap();
      match commodities.get_mut(*self) {
         Some(com) => Ok(f(com)),
         None => anyhow::bail!("commodity not found"),
      }
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
      match Commodity::new(&ctx, base.join(f)) {
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

      com.c = CommodityC::new(com)?;

      Ok(())
   }

   // Second pass - set up references and C stuff
   for (id, com) in data.iter_mut() {
      let comload = load.get(id).unwrap();
      if let Err(e) = update_commodity(com, comload, &commap) {
         warn_err!(e);
      }
   }

   Ok(())
}

impl FromLua for CommodityRef {
   fn from_lua(value: mlua::Value, _: &mlua::Lua) -> mlua::Result<Self> {
      match value {
         mlua::Value::UserData(ud) => Ok(*ud.borrow::<Self>()?),
         mlua::Value::String(name) => {
            let name = &name.to_str()?;
            Ok(CommodityRef::new(name).with_context(|| format!("commodity '{name}' not found"))?)
         }
         val => Err(mlua::Error::RuntimeError(format!(
            "unable to convert {} to CommodityRef",
            val.type_name()
         ))),
      }
   }
}

/*@
 * @brief Lua bindings to interact with commodities.
 *
 * This will allow you to create and manipulate commodities in-game.
 *
 * An example would be:
 * @code
 * c = commodity.get( "Food" ) -- Gets the commodity by name
 * if c:price() > 500 then
 *    -- Do something with high price
 * end
 * @endcode
 *
 * @lua_mod commodity
 */
impl UserData for CommodityRef {
   /*@
    * @brief Gets the translated name of the commodity.
    *    @luatparam Commodity c Commodity to get the translated name of.
    *    @luatreturn string The translated name of the commodity.
    * @luafunc __tostring
    * @see name
    */
   fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
      methods.add_meta_function(MetaMethod::ToString, |_, this: Self| {
         Ok(this.call(|com| com.name().to_string())?)
      });
      /*@
       * @brief Checks to see if two commodities are the same.
       *
       * @usage if c1 == c2 then -- Checks to see if commodity c1 and c2 are the same
       *
       *    @luatparam Commodity c1 First commodity to compare.
       *    @luatparam Commodity c2 Second commodity to compare.
       *    @luatreturn boolean true if both commodities are the same.
       * @luafunc __eq
       */
      methods.add_meta_function(
         MetaMethod::Eq,
         |_, (this, other): (Self, Self)| -> mlua::Result<bool> { Ok(this == other) },
      );
      /*@
       * @brief Gets a commodity.
       *
       * @usage s = commodity.get( "Food" ) -- Gets the food commodity
       *
       *    @luatparam string s Raw (untranslated) name of the commodity to get.
       *    @luatreturn Commodity|nil The commodity matching name or nil if error.
       * @luafunc get
       */
      methods.add_function("get", |_, name: BorrowedStr| -> mlua::Result<Self> {
         Ok(CommodityRef::new(&name).with_context(|| format!("Commodity '{name}' not found!"))?)
      });
      /*@
       * @brief Gets a table with all the commodities.
       *
       * @usage for k,v in ipairs( commodity.getAll() ) do ... end
       *
       *    @luatreturn {Commodity} Ordered list of commodities.
       * @luafunc getAll
       */
      methods.add_function("getAll", |_, ()| -> mlua::Result<Vec<Self>> {
         Ok(COMMODITIES.read().unwrap().keys().collect())
      });
      /*@
       * @brief Gets a commodity if it exists.
       *
       * @usage s = commodity.exists( "Food" ) -- Gets the food commodity
       *
       *    @luatparam string s Raw (untranslated) name of the commodity to get.
       *    @luatreturn Commodity|nil The commodity matching name or nil if error.
       * @luafunc exists
       */
      methods.add_function(
         "exists",
         |_, name: BorrowedStr| -> mlua::Result<Option<Self>> { Ok(CommodityRef::new(&name)) },
      );
      /*@
       * @brief Gets the list of standard commodities.
       *
       *    @luatreturn table A table containing commodity objects, namely those which
       * are standard (buyable/saleable anywhere).
       * @luafunc getStandard
       */
      methods.add_function("getStandard", |_, ()| -> mlua::Result<Vec<Self>> {
         Ok(COMMODITIES
            .read()
            .unwrap()
            .iter()
            .filter_map(|(k, v)| v.standard.then_some(k))
            .collect())
      });
      /*@
       * @brief Gets the translated name of the commodity.
       *
       * This translated name should be used for display purposes (e.g.
       * messages). It cannot be used as an identifier for the commodity; for
       * that, use commodity.nameRaw() instead.
       *
       * It is not necessarily equivalent to _(c:name()) if the commodity has a different display name.
       *
       * @usage commodityname = c:name()
       *
       *    @luatparam Commodity c Commodity to get the translated name of.
       *    @luatreturn string The translated name of the commodity.
       * @luafunc name
       */
      methods.add_method("name", |_, this, ()| -> mlua::Result<String> {
         Ok(this.call(|com| com.name().to_string())?)
      });
      /*@
       * @brief Gets the raw (untranslated) name of the commodity.
       *
       * This untranslated name should be used for identification purposes
       * (e.g. can be passed to commodity.get()). It should not be used
       * directly for display purposes without manually translating it with
       * _().
       *
       * @usage commodityrawname = c:nameRaw()
       *
       *    @luatparam Commodity c Commodity to get the raw name of.
       *    @luatreturn string The raw name of the commodity.
       * @luafunc nameRaw
       */
      methods.add_method("name_raw", |_, this, ()| -> mlua::Result<String> {
         Ok(this.call(|com| com.name.to_string())?)
      });
      /*@
       * @brief Gets the base price of an commodity.
       *
       * @usage print( c:price() ) -- Prints the base price of the commodity
       *
       *    @luatparam Commodity c Commodity to get information of.
       *    @luatreturn number The base price of the commodity.
       * @luafunc price
       */
      methods.add_method("price", |_, this, ()| -> mlua::Result<i64> {
         Ok(this.call(|com| com.price)?)
      });
      /*@
       * @brief Gets the base price of an commodity on a certain spob.
       *
       * @usage if c:priceAt( spob.get("Polaris Prime") ) > 100 then -- Checks price
       * of a commodity at Polaris Prime
       *
       *    @luatparam Commodity c Commodity to get information of.
       *    @luatparam Spob p Spob to get price at.
       *    @luatreturn number The price of the commodity at the spob.
       * @luafunc priceAt
       */
      methods.add_method("priceAt", |_, this, ()| -> mlua::Result<i64> {
         Ok(this.call(|com| com.price)?)
      });
      /*@
       * @brief Gets the price of an commodity on a certain spob at a certain time.
       *
       * @usage if c:priceAtTime( spob.get("Polaris Prime"), time ) > 100 then --
       * Checks price of a commodity at Polaris Prime
       *
       *    @luatparam Commodity c Commodity to get information of.
       *    @luatparam Spob p Spob to get price at.
       *    @luatparam Time t Time to get the price at.
       *    @luatreturn number The price of the commodity at the spob.
       * @luafunc priceAtTime
       */
      methods.add_method("priceAtTime", |_, this, ()| -> mlua::Result<i64> {
         Ok(this.call(|com| com.price)?)
      });
      /*@
       * @brief Gets the store icon of a commodity if it exists.
       *
       *    @luatparam Commodity c Commodity to get icon of.
       *    @luatreturn Texture|nil Texture of the store icon if exists, otherwise
       * nil.
       * @luafunc icon
       */
      methods.add_method(
         "logo",
         |_, this, ()| -> mlua::Result<Option<texture::Texture>> {
            Ok(this
               .call(|fct| fct.gfx_store.as_ref().map(|t| t.try_clone()))?
               .transpose()?)
         },
      );
      /*@
       * @brief Gets the description of a commodity if it exists.
       *
       *    @luatparam Commodity c Commodity to get description of
       *    @luatreturn string|nil Description of the commodity if exists, otherwise
       * nil.
       * @luafunc description
       */
      methods.add_method("name", |_, this, ()| -> mlua::Result<String> {
         Ok(this.call(|com| com.description.to_string())?)
      });
      /*@
       * @brief Creates a new temporary commodity. If a temporary commodity with the
       * same name exists, that gets returned instead. "Temporary" is a relative term.
       * The commodity will be saved with the player while it is in the inventory of
       * their fleet. However, when all instances are gone, it will no longer be saved
       * and disappear.
       *
       * @usage commodity.new( N_("Cheesburgers"), N_("I can has cheezburger?") )
       *
       *    @luatparam string cargo Name of the cargo to add. This must not match a
       * cargo name defined in commodity.xml.
       *    @luatparam string decription Description of the cargo to add.
       *    @luatparam[opt=nil] table params Table of named parameters. Currently
       * supported is `"gfx_space"`.
       *    @luatreturn Commodity The newly created commodity or an existing temporary
       * commodity with the same name.
       * @luafunc new
       */
      methods.add_function(
         "new",
         |_, (name, desc, params): (BorrowedStr, BorrowedStr, mlua::Table)| -> mlua::Result<Self> {
            // Handle parameters
            let gfx_space = params
               .get::<Option<UserDataRef<texture::Texture>>>("gfx_space")?
               .map(|t| t.try_clone())
               .transpose()?;

            // Add the commodity
            Ok(COMMODITIES.write().unwrap().insert_with_key(|k| {
               let mut com = Commodity {
                  id: k,
                  name: name.to_string(),
                  description: desc.to_string(),
                  gfx_space: gfx_space,
                  temporary: true,
                  ..Default::default()
               };
               if let Ok(c) = CommodityC::new(&com) {
                  com.c = c;
               }
               com
            }))
         },
      );
      /*@
       * @brief Makes a temporary commodity illegal to a faction.
       *
       *    @luatparam Commodity c Temporary commodity to make illegal to factions.
       *    @luatparam Faction|table f Faction or table of factions to make illegal
       * to.
       * @luafunc illegalto
       */
      methods.add_method(
         "illegalto",
         |_, this, fct: Either<FactionRef, Vec<FactionRef>>| -> mlua::Result<()> {
            this.call_mut(|com| {
               if !com.temporary {
                  return anyhow::bail!("Trying to modify non-temporary commodity '{}'", com.name);
               }
               match &fct {
                  Either::Left(f) => com.illegal_to.push(*f),
                  Either::Right(fcts) => com.illegal_to.extend(fcts),
               }
               Ok(())
            })?;
            Ok(())
         },
      );
      /*@
       * @brief Gets the factions to which the commodity is illegal to.
       *
       *    @luatparam c Commodity to get illegality status of.
       *    @luatreturn table Table of all the factions the commodity is illegal to.
       * @luafunc illegality
       */
      methods.add_method(
         "illegality",
         |_, this, ()| -> mlua::Result<Vec<FactionRef>> {
            Ok(this.call(|com| com.illegal_to.clone())?)
         },
      );
      /*@
       * @brief Gets the commodity tags.
       *
       *    @luatparam c Commodity to get tags of.
       *    @luatparam[opt=nil] string tag Tag to check if exists.
       *    @luatreturn table|boolean Table of tags where the name is the key and true
       * is the value or a boolean value if a string is passed as the second parameter
       * indicating whether or not the specified tag exists.
       * @luafunc tags
       */
      methods.add_method("tags", |lua, this, ()| -> mlua::Result<mlua::Table> {
         this.call(|com| lua.create_table_from(com.tags.iter().map(|s| (s.clone(), true))))?
      });
   }
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
fn faction_c_call_mut<F, R>(id: i64, f: F) -> Result<R>
where
   F: Fn(&mut Commodity) -> R,
{
   let mut commodities = COMMODITIES.write().unwrap();
   match commodities.get_mut(CommodityRef::from_ffi(id)) {
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

#[unsafe(no_mangle)]
pub extern "C" fn _commodity_always_can_sell(com: i64) -> c_int {
   faction_c_call(com, |c| c.always_can_sell as c_int).unwrap_or_else(|e| {
      warn_err!(e);
      0
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn _commodity_isTemp(com: i64) -> c_int {
   faction_c_call(com, |c| c.temporary as c_int).unwrap_or_else(|e| {
      warn_err!(e);
      0
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn _commodity_price_ref(com: i64) -> i64 {
   faction_c_call(com, |c| match &c.price_ref {
      Some(pr) => pr.base.as_ffi(),
      None => CommodityRef::null().as_ffi(),
   })
   .unwrap_or_else(|e| {
      warn_err!(e);
      CommodityRef::null().as_ffi()
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn _commodity_price_mod(com: i64) -> c_double {
   faction_c_call(com, |c| match &c.price_ref {
      Some(pr) => pr.modifier as c_double,
      None => 1.0,
   })
   .unwrap_or_else(|e| {
      warn_err!(e);
      1.0
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn _commodity_last_purchase_price(com: i64) -> i64 {
   faction_c_call(com, |c| c.last_purchased_price.load(Ordering::Relaxed)).unwrap_or_else(|e| {
      warn_err!(e);
      0
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn _commodity_set_last_purchase_price(com: i64, amount: i64) {
   faction_c_call(com, |c| {
      c.last_purchased_price.store(amount, Ordering::Relaxed)
   })
   .unwrap_or_else(|e| {
      warn_err!(e);
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn _commodity_checkIllegal(com: i64, fct: i64) -> c_int {
   faction_c_call(com, |c| {
      c.illegal_to.contains(&FactionRef::from_ffi(fct)) as c_int
   })
   .unwrap_or_else(|e| {
      warn_err!(e);
      0
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn _commodity_newTemp(name: *const c_char, desc: *const c_char) -> i64 {
   let ptr = unsafe { CStr::from_ptr(name) };
   let name = ptr.to_str().unwrap().to_string();
   let ptr = unsafe { CStr::from_ptr(desc) };
   let desc = ptr.to_str().unwrap().to_string();
   COMMODITIES
      .write()
      .unwrap()
      .insert_with_key(|k| {
         let mut com = Commodity {
            id: k,
            name,
            description: desc,
            ..Default::default()
         };
         if let Ok(c) = CommodityC::new(&com) {
            com.c = c;
         }
         com
      })
      .as_ffi()
}

#[unsafe(no_mangle)]
pub extern "C" fn _commodity_tempIllegalto(com: i64, fct: i64) -> c_int {
   faction_c_call_mut(com, |c| {
      c.illegal_to.push(FactionRef::from_ffi(fct));
      if let Ok(a) = Array::new(&c.illegal_to) {
         c.c.illegal_to = a;
      }
      0
   })
   .unwrap_or_else(|e| {
      warn_err!(e);
      -1
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn _standard_commodities() -> *const i64 {
   static STANDARD_COMMODITIES: LazyLock<Array<i64>> = LazyLock::new(|| {
      let std: Vec<_> = COMMODITIES
         .read()
         .unwrap()
         .iter()
         .filter_map(|(k, v)| if v.standard { Some(k.as_ffi()) } else { None })
         .collect();
      Array::new(&std).unwrap()
   });
   STANDARD_COMMODITIES.as_ptr() as *const i64
}
