use mlua::{FromLua, Lua, MetaMethod, UserData, UserDataMethods, Value};
use nalgebra::Vector4;
use palette::FromColor;
use palette::{Hsv, LinSrgb, Srgb};

#[derive(Copy, Clone, derive_more::From, derive_more::Into)]
pub struct Colour(Vector4<f64>);

impl Colour {
    pub const fn new(r: f64, g: f64, b: f64, a: f64) -> Self {
        Colour(Vector4::new(r, g, b, a))
    }
    pub const fn new_rgb(r: f64, g: f64, b: f64) -> Self {
        Colour::new(r, g, b, 1.0)
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
            |_, (r, g, b, a): (f64, f64, f64, Option<f64>)| -> mlua::Result<Self> {
                let a = a.unwrap_or(1.0);
                Ok(Colour::new(r, g, b, a))
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
             (h, s, v, a, gamma): (f64, f64, f64, Option<f64>, Option<bool>)|
             -> mlua::Result<Self> {
                let a = a.unwrap_or(1.0);
                let gamma = gamma.unwrap_or(false);
                let col = Srgb::from_color(Hsv::new(h, s, v));
                let (r, g, b) = if gamma {
                    col.into_linear().into_components()
                } else {
                    col.into_components()
                };
                Ok(Colour::new(r, g, b, a))
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
        methods.add_method("alpha", |_, this, ()| -> mlua::Result<f64> { Ok(this.0.w) });
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
            |_, this, gamma: Option<bool>| -> mlua::Result<(f64, f64, f64)> {
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
            |_, this, gamma: Option<bool>| -> mlua::Result<(f64, f64, f64, f64)> {
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
        methods.add_method("hsv", |_, this, ()| -> mlua::Result<(f64, f64, f64)> {
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
            |_, this, (r, g, b): (f64, f64, f64)| -> mlua::Result<()> {
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
            |_, this, (h, s, v): (f64, f64, f64)| -> mlua::Result<()> {
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
        methods.add_method_mut("setAlpha", |_, this, a: f64| -> mlua::Result<()> {
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
            Ok(Colour::new(r, g, b, this.0.w))
        });
        /*
         * @brief Converts a colour from gamma corrected to linear.
         *
         *    @luatparam Colour col Colour to change from gamma corrected to linear.
         *    @luatreturn Colour Modified colour.
         * @luafunc gammaToLinear
         */
        methods.add_method("gammaToLinear", |_, this, ()| -> mlua::Result<Colour> {
            let (r, g, b) =
                LinSrgb::from_color(Srgb::new(this.0.x, this.0.y, this.0.z)).into_components();
            Ok(Colour::new(r, g, b, this.0.w))
        });
    }
}

pub fn open_colour(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData> {
    let proxy = lua.create_proxy::<Colour>()?;
    Ok(proxy)
}
