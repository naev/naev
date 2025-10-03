use mlua::{FromLua, Lua, MetaMethod, UserData, UserDataMethods, Value};
use nalgebra::Vector4;
use palette::FromColor;
use palette::{Hsv, LinSrgb, Srgb};
use std::sync::LazyLock;
use trie_rs::map::{Trie, TrieBuilder};

#[derive(Copy, Clone, derive_more::From, derive_more::Into, PartialEq)]
pub struct Colour(Vector4<f32>);

macro_rules! colour {
    ($name: ident, $r: literal, $g: literal, $b: literal ) => {
        pub const $name: Colour = Colour::from_gamma_const($r, $g, $b);
    };
    ($name: ident, $r: literal, $g: literal, $b: literal, $a: literal ) => {
        pub const $name: Colour = Colour::from_gamma_alpha_const($r, $g, $b, $a);
    };
}

macro_rules! colours {
    ($( $name: ident => ($r: expr, $g: expr, $b: expr ) ),*) => {
        $(pub const $name: Colour = Colour::from_gamma_const($r, $g, $b);)*
        static LOOKUP: LazyLock<Trie<u8, Colour>> = LazyLock::new(|| {
            let mut builder = TrieBuilder::new();
            $( builder.push(stringify!($name).to_lowercase(), $name); )*
            builder.build()
        });
    };
}

// Colour constants
colours![
    WHITE  => (1.0, 1.0, 1.0),
    GREY90 => (0.9, 0.9, 0.9),
    GREY80 => (0.8, 0.8, 0.8),
    GREY70 => (0.7, 0.7, 0.7),
    GREY60 => (0.6, 0.6, 0.6),
    GREY50 => (0.5, 0.5, 0.5),
    GREY40 => (0.4, 0.4, 0.4),
    GREY30 => (0.3, 0.3, 0.3),
    GREY20 => (0.2, 0.2, 0.2),
    GREY10 => (0.1, 0.1, 0.1),
    BLACK  => (0.0, 0.0, 0.0),
    // Greens
    DARKGREEN  => (0.1, 0.5, 0.),
    GREEN      => (0.2, 0.8, 0.2),
    PRIMEGREEN => (0.0, 1.0, 0.0),
    // Reds
    DARKRED   => (0.6, 0.1, 0.1),
    RED       => (0.8, 0.2, 0.2),
    BRIGHTRED => (1.0, 0.6, 0.6),
    PRIMERED  => (1.0, 0.0, 0.0),
    // Oranges
    ORANGE => (0.8, 0.7, 0.1),
    // Yellows
    GOLD   => (1.0, 0.84, 0.0),
    YELLOW => (0.8, 0.8, 0.0),
    // Blue
    MIDNIGHTBLUE => (0.1, 0.1, 0.4),
    DARKBLUE     => (0.1, 0.1, 0.6),
    BLUE         => (0.2, 0.2, 0.8),
    AQUABLUE     => (0.3, 0.3, 0.9),
    AQUA         => (0.0, 0.75, 1.0),
    LIGHTBLUE    => (0.4, 0.4, 1.0),
    CYAN         => (0.0, 1.0, 1.0),
    PRIMEBLUE    => (0.0, 0.0, 1.0),
    // Purples
    PURPLE     => (0.9, 0.1, 0.9),
    DARKPURPLE => (0.68, 0.18, 0.64),
    // Browns
    BROWN => (0.59, 0.28, 0.0),
    // Misc.
    SILVER => (0.75, 0.75, 0.75),

    // Outfit slot colours
    // Taken from https://cran.r-project.org/web/packages/khroma/vignettes/tol.html#muted
    OUTFITHEAVY  => ( 0.8, 0.4, 0.46 ),
    OUTFITMEDIUM => (0.16, 0.63, 0.81 ),
    OUTFITLIGHT  => ( 0.75, 0.7, 0.4 ),
    // Objects
    INERT      => (221./255., 221./255., 221./255.),
    NEUTRAL    => (221./255., 204./255., 119./255.),
    FRIEND     => (68./255., 170./255., 153./255.),
    HOSTILE    => (204./255.,  68./255., 153./255.),
    RESTRICTED => (221./255., 153./255.,  51./255.),
    // Mission Markers
    // https://packages.tesselle.org/khroma/articles/tol.html#vibrant
    MARKERNEW      => (51./255., 187./255., 238./255.),
    MARKERCOMPUTER => (51./255., 187./255., 238./255.),
    MARKERLOW      => (0./255., 153./255., 136./255.),
    MARKERHIGH     => (238./255., 119./255., 51./255.),
    MARKERPLOT     => (238./255., 51./255., 119./255.),
   // Radar
    RADARPLAYER   => (0.9, 0.1, 0.9),
    RADARTARGET   => (1.0, 1.0, 1.0),
    RADARWEAPON   => (0.8, 0.2, 0.2),
    RADARHILIGHT  => (0.6, 1.0, 1.0),
    RADARSCANNING => (1.0, 1.0, 0.6),
    //RADARVIEWPORT => (  1.0, 1.0, 1.0 ), //, 0.5),
   // HEALTH
    SHIELD => (0.2, 0.2, 0.8),
    ARMOUR => (0.5, 0.5, 0.5),
    ENERGY => (0.2, 0.8, 0.2),
    FUEL   => (0.9, 0.1, 0.4),
   // DEIZ'S SUPER FONT PALETTE
    FONTRED    => (1.0, 0.4, 0.4),
    FONTGREEN  => (0.6, 1.0, 0.4),
    FONTBLUE   => (0.4, 0.6, 1.0),
    FONTYELLOW => (1.0, 1.0, 0.5),
    FONTGREY   => (0.7, 0.7, 0.7),
    FONTPURPLE => (1.0, 0.3, 1.0),
    FONTORANGE => (1.0, 0.7, 0.3),
    FONTWHITE  => (0.95, 0.95, 0.95)
];
// Special with transparency (not accessible from names)
colour!(TRANSPARENT, 0.0, 0.0, 0.0, 0.0);
colour!(BLACKHILIGHT, 0.0, 0.0, 0.0, 0.4); // Highlight colour over black background
colour!(RADARVIEWPORT, 1.0, 1.0, 1.0, 0.5);

/*
static LOOKUP: LazyLock<Trie<u8, Colour>> = LazyLock::new(|| {
    let mut builder = TrieBuilder::new();
    builder.push("white", WHITE);
    builder.push("black", BLACK);
    builder.build()
});
*/

/// softfloat doesn't have a native `powf` so we just approximate it
const fn powf_const(a: f32, b: f32) -> f32 {
    // Have to jump through hoops as there is no F32::exp nor F32::ln
    let a = softfloat::F64::from_f32(softfloat::F32::from_native_f32(a));
    let b = softfloat::F64::from_f32(softfloat::F32::from_native_f32(b));
    softfloat::F64::exp(b.mul(a.ln())).to_f32().to_native_f32()
}

#[test]
fn test_soft_powf() {
    let a: f32 = 0.5;
    assert!(
        (a.powf(2.4) - powf_const(a, 2.4)).abs() < 1e-9,
        "softfloat powf"
    );
}

/// Constant implementation of Gamma to Linear transformation for use with declaring new colours
const fn gam_to_lin_const(x: f32) -> f32 {
    if x <= 0.04045 {
        1.0 / 12.92 * x
    } else {
        powf_const((x + 0.055) / 1.055, 2.4)
    }
}

impl Colour {
    pub const fn new(r: f32, g: f32, b: f32) -> Self {
        Colour::new_alpha(r, g, b, 1.0)
    }
    pub const fn new_alpha(r: f32, g: f32, b: f32, a: f32) -> Self {
        Colour(Vector4::new(r, g, b, a))
    }
    pub fn from_gamma(r: f32, g: f32, b: f32) -> Self {
        Colour::from_gamma_alpha(r, g, b, 1.0)
    }
    pub fn from_gamma_alpha(r: f32, g: f32, b: f32, a: f32) -> Self {
        let (r, g, b) = Srgb::new(r, g, b).into_linear().into_components();
        Colour::new_alpha(r, g, b, a)
    }
    pub const fn from_gamma_const(r: f32, g: f32, b: f32) -> Self {
        Colour::from_gamma_alpha_const(r, g, b, 1.0)
    }
    pub const fn from_gamma_alpha_const(r: f32, g: f32, b: f32, a: f32) -> Self {
        Colour(Vector4::new(
            gam_to_lin_const(r),
            gam_to_lin_const(g),
            gam_to_lin_const(b),
            a,
        ))
    }

    pub fn from_name(name: &str) -> Option<Self> {
        LOOKUP.exact_match(name.to_lowercase()).copied()
    }
}

impl FromLua for Colour {
    fn from_lua(value: Value, _: &Lua) -> mlua::Result<Self> {
        match value {
            Value::UserData(ud) => Ok(*ud.borrow::<Self>()?),
            Value::String(_name) => todo!(),
            val => Err(mlua::Error::RuntimeError(format!(
                "unable to convert {} to Colour",
                val.type_name()
            ))),
        }
    }
}

/*
 * @brief Lua bindings to interact with colours.
 *
 * An example would be:
 * @code
 * col1 = colour.new( "Red" ) -- Get by name
 * col2 = colour.new( 0.5, 0.5, 0.5, 0.3 ) -- Create with RGB values
 * col3 = colour.new() -- White by default
 * col2:setHSV( col1:hsv() ) -- Set colour 2 with colour 1's HSV values
 * @endcode
 *
 * Colours are assumed to be given as gamma-corrected values and are stored
 * internally in linear colour space by default. Most functions have a boolean
 * parameter that allows controlling this behaviour.
 *
 * @luamod colour
 */
impl UserData for Colour {
    fn add_fields<F: mlua::UserDataFields<Self>>(fields: &mut F) {
        fields.add_field_method_get("r", |_, this| Ok(this.0.x));
        fields.add_field_method_get("g", |_, this| Ok(this.0.y));
        fields.add_field_method_get("b", |_, this| Ok(this.0.z));
        fields.add_field_method_get("a", |_, this| Ok(this.0.w));
    }

    fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
        /*
         * @brief Converts a colour to a string.
         *
         *    @luatparam Colour col Colour to get string from.
         *    @luatreturn string A string representing the colour.
         * @luafunc __tostring
         */
        methods.add_meta_method(
            MetaMethod::ToString,
            |_, this, ()| -> mlua::Result<String> {
                Ok(format!(
                    "Colour( {}, {}, {}, {} )",
                    this.0.x, this.0.y, this.0.z, this.0.w
                ))
            },
        );
        /*
         * @brief Compares two colours to see if they are the same.
         *
         *    @luatparam Colour c1 Colour 1 to compare.
         *    @luatparam Colour c2 Colour 2 to compare.
         *    @luatreturn boolean true if both colours are the same.
         * @luafunc __eq
         */
        methods.add_meta_method(
            MetaMethod::Eq,
            |_, this, other: Value| -> mlua::Result<bool> {
                match other {
                    Value::UserData(ud) => match ud.borrow::<Self>() {
                        Ok(other) => Ok(*this == *other),
                        Err(_) => Ok(false),
                    },
                    _ => Ok(false),
                }
            },
        );
        /*
         * @brief Creates a new colour. Colours are assumed to be in gamma colour space
         * by default and are converted to linear unless specified.
         *
         * @usage colour.new( "Red" ) -- Gets colour by name
         * @usage colour.new( "Red", 0.5 ) -- Gets colour by name with alpha 0.5
         * @usage colour.new() -- Creates a white (blank) colour
         * @usage colour.new( 1., 0., 0. ) -- Creates a bright red colour
         * @usage colour.new( 1., 0., 0., 0.5 ) -- Creates a bright red colour with
         * alpha 0.5
         *
         *    @luatparam number r Red value of the colour.
         *    @luatparam number g Green value of the colour.
         *    @luatparam number b Blue value of the colour.
         *    @luatparam[opt=1.] number a Alpha value of the colour.
         *    @luatparam[opt=false] gamma Whether to load the colour in the gamma
         * colour space.
         *    @luatreturn Colour A newly created colour.
         * @luafunc new
         */
        methods.add_function(
            "new",
            |_,
             (r, g, b, a, gamma): (f32, f32, f32, Option<f32>, Option<bool>)|
             -> mlua::Result<Self> {
                let a = a.unwrap_or(1.0);
                let gamma = gamma.unwrap_or(false);
                if gamma {
                    Ok(Colour::new_alpha(r, g, b, a))
                } else {
                    let (r, g, b) = Srgb::new(r, g, b).into_linear().into_components();
                    Ok(Colour::new_alpha(r, g, b, a))
                }
            },
        );
        /*
         * @brief Creates a new colour from HSV values. Colours are assumed to be in
         * gamma colour space by default and are converted to linear unless specified.
         *
         * @usage colour.new( 0., 0.5, 0.5 ) -- Creates a colour with 0 hue, 0.5
         * saturation and 0.5 value.
         *
         *    @luatparam number h Hue of the colour (0-360 value).
         *    @luatparam number s Saturation of the colour (0-1 value).
         *    @luatparam number v Value of the colour (0-1 value).
         *    @luatparam[opt=1.] number a Alpha value of the colour.
         *    @luatparam[opt=false] gamma Whether to load the colour in the gamma
         * colour space.
         *    @luatreturn Colour A newly created colour.
         * @luafunc newHSV
         */
        methods.add_function(
            "newHSV",
            |_,
             (h, s, v, a, gamma): (f32, f32, f32, Option<f32>, Option<bool>)|
             -> mlua::Result<Self> {
                let a = a.unwrap_or(1.0);
                let gamma = gamma.unwrap_or(false);
                let col = Srgb::from_color(Hsv::new(h, s, v));
                let (r, g, b) = if gamma {
                    col.into_linear().into_components()
                } else {
                    col.into_components()
                };
                Ok(Colour::new_alpha(r, g, b, a))
            },
        );
        /*
         * @brief Gets the alpha of a colour.
         *
         * Value is from from 0. (transparent) to 1. (opaque).
         *
         * @usage colour_alpha = col:alpha()
         *
         *    @luatparam Colour col Colour to get alpha of.
         *    @luatreturn number The alpha of the colour.
         * @luafunc alpha
         */
        methods.add_method("alpha", |_, this, ()| -> mlua::Result<f32> { Ok(this.0.w) });
        /*
         * @brief Gets the RGB values of a colour.
         *
         * Values are from 0. to 1.
         *
         * @usage r,g,b = col:rgb()
         *
         *    @luatparam Colour col Colour to get RGB values of.
         *    @luatparam[opt=false] boolean gamma Whether or not to get the
         * gamma-corrected value or not. Defaults to internal value that is linear.
         *    @luatreturn number The red value of the colour.
         *    @luatreturn number The green value of the colour.
         *    @luatreturn number The blue value of the colour.
         * @luafunc rgb
         */
        methods.add_method(
            "rgb",
            |_, this, gamma: Option<bool>| -> mlua::Result<(f32, f32, f32)> {
                let gamma = gamma.unwrap_or(false);
                let (r, g, b) = if gamma {
                    Srgb::from_linear(LinSrgb::new(this.0.x, this.0.y, this.0.z)).into_components()
                } else {
                    (this.0.x, this.0.y, this.0.z)
                };
                Ok((r, g, b))
            },
        );
        /*
         * @brief Gets the RGBA values of a colour.
         *
         * Values are from 0. to 1.
         *
         * @usage r,g,b,a = col:rgba()
         *
         *    @luatparam Colour col Colour to get RGB values of.
         *    @luatparam[opt=false] boolean gamma Whether or not to get the
         * gamma-corrected value or not.
         *    @luatreturn number The red value of the colour.
         *    @luatreturn number The green value of the colour.
         *    @luatreturn number The blue value of the colour.
         *    @luatreturn number The alpha value of the colour.
         * @luafunc rgba
         */
        methods.add_method(
            "rgba",
            |_, this, gamma: Option<bool>| -> mlua::Result<(f32, f32, f32, f32)> {
                let gamma = gamma.unwrap_or(false);
                let (r, g, b) = if gamma {
                    Srgb::from_linear(LinSrgb::new(this.0.x, this.0.y, this.0.z)).into_components()
                } else {
                    (this.0.x, this.0.y, this.0.z)
                };
                Ok((r, g, b, this.0.w))
            },
        );
        /*
         * @brief Gets the HSV values of a colour.
         *
         * Values are from 0 to 1 except hue which is 0 to 360.
         *
         * @usage h,s,v = col:rgb()
         *
         *    @luatparam Colour col Colour to get HSV values of.
         *    @luatreturn number The hue of the colour (0-360 value).
         *    @luatreturn number The saturation of the colour (0-1 value).
         *    @luatreturn number The value of the colour (0-1 value).
         * @luafunc hsv
         */
        methods.add_method("hsv", |_, this, ()| -> mlua::Result<(f32, f32, f32)> {
            let hsv = Hsv::from_color(LinSrgb::new(this.0.x, this.0.y, this.0.z));
            let (h, s, v) = hsv.into_components();
            Ok((h.into(), s, v))
        });
        /*
         * @brief Sets the colours values from the RGB colour space.
         *
         * Values are from 0. to 1.
         *
         * @usage col:setRGB( r, g, b )
         *
         *    @luatparam Colour col Colour to set RGB values.
         *    @luatparam number r Red value to set.
         *    @luatparam number g Green value to set.
         *    @luatparam number b Blue value to set.
         * @luafunc setRGB
         */
        methods.add_method_mut(
            "setRGB",
            |_, this, (r, g, b): (f32, f32, f32)| -> mlua::Result<()> {
                this.0.x = r;
                this.0.y = g;
                this.0.z = b;
                Ok(())
            },
        );
        /*
         * @brief Sets the colours values from the HSV colour space.
         *
         * Values are from 0. to 1.
         *
         * @usage col:setHSV( h, s, v )
         *
         *    @luatparam Colour col Colour to set HSV values.
         *    @luatparam number h Hue value to set.
         *    @luatparam number s Saturation value to set.
         *    @luatparam number v Value to set.
         * @luafunc setHSV
         */
        methods.add_method_mut(
            "setHSV",
            |_, this, (h, s, v): (f32, f32, f32)| -> mlua::Result<()> {
                let (r, g, b) = Srgb::from_color(Hsv::new(h, s, v)).into_components();
                this.0.x = r;
                this.0.y = g;
                this.0.z = b;
                Ok(())
            },
        );
        /*
         * @brief Sets the alpha of a colour.
         *
         * Value is from 0. (transparent) to 1. (opaque).
         *
         * @usage col:setAlpha( 0.5 ) -- Make colour half transparent
         *
         *    @luatparam Colour col Colour to set alpha of.
         *    @luatparam number alpha Alpha value to set.
         * @luafunc setAlpha
         */
        methods.add_method_mut("setAlpha", |_, this, a: f32| -> mlua::Result<()> {
            this.0.w = a;
            Ok(())
        });
        /*
         * @brief Converts a colour from linear to gamma corrected.
         *
         *    @luatparam Colour col Colour to change from linear to gamma.
         *    @luatreturn Colour Modified colour.
         * @luafunc linearToGamma
         */
        methods.add_method("linearToGamma", |_, this, ()| -> mlua::Result<Colour> {
            let (r, g, b) =
                Srgb::from_linear(LinSrgb::new(this.0.x, this.0.y, this.0.z)).into_components();
            Ok(Colour::new_alpha(r, g, b, this.0.w))
        });
        /*
         * @brief Converts a colour from gamma corrected to linear.
         *
         *    @luatparam Colour col Colour to change from gamma corrected to linear.
         *    @luatreturn Colour Modified colour.
         * @luafunc gammaToLinear
         */
        methods.add_method("gammaToLinear", |_, this, ()| -> mlua::Result<Colour> {
            let (r, g, b) = Srgb::new(this.0.x, this.0.y, this.0.z)
                .into_linear()
                .into_components();
            Ok(Colour::new_alpha(r, g, b, this.0.w))
        });
    }
}

pub fn open_colour(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData> {
    let proxy = lua.create_proxy::<Colour>()?;
    Ok(proxy)
}

#[test]
fn test_mlua_file() {
    let lua = mlua::Lua::new();
    let globals = lua.globals();
    globals.set("colour", open_colour(&lua).unwrap()).unwrap();
    lua.load(
        r#"
local lin = colour.new( 0.5, 0.5, 0.5 )
local gam = colour.new( 0.5, 0.5, 0.5, nil, true )
assert( lin ~= gam, "gamma and linear are identical" )
assert( lin == gam:gammaToLinear(), "gammaToLinear() failed" )
assert( gam == lin:linearToGamma(), "linearToGamma() failed" )
assert( lin == lin:linearToGamma():gammaToLinear(), "roundtrip linear -> gamma -> linear failed" )
        "#,
    )
    .set_name("mlua Colour test")
    .exec()
    .unwrap();
}
