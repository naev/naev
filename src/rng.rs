use mlua::{Either, UserData, UserDataMethods};
use rand::Rng;
use std::os::raw::{c_double, c_uint};

#[unsafe(no_mangle)]
pub unsafe extern "C" fn randint() -> c_uint {
    rng::<c_uint>()
}
#[unsafe(no_mangle)]
pub unsafe extern "C" fn randfp() -> c_double {
    rng::<c_double>()
}
#[unsafe(no_mangle)]
pub unsafe extern "C" fn Normal(x: c_double) -> c_double {
    normal(x)
}
#[unsafe(no_mangle)]
pub unsafe extern "C" fn NormalInverse(p: c_double) -> c_double {
    normal_inverse(p) as c_double
}

thread_local! {
    static RNG: std::cell::RefCell<rand::rngs::ThreadRng> = std::cell::RefCell::new(rand::rng());
}

pub fn rng<T>() -> T
where
    rand::distr::StandardUniform: rand::prelude::Distribution<T>,
{
    RNG.with_borrow_mut(|x| x.random::<T>())
}

pub fn range<T, R>(range: R) -> T
where
    T: std::cmp::PartialOrd + rand::distr::uniform::SampleUniform,
    R: rand::distr::uniform::SampleRange<T>,
{
    RNG.with_borrow_mut(|x| x.random_range(range))
}

// Taken from probability package.
#[allow(clippy::excessive_precision)]
pub fn normal_inverse(p: f64) -> f64 {
    //should!((0.0..=1.0).contains(&p));

    const CONST1: f64 = 0.180625;
    const CONST2: f64 = 1.6;
    const SPLIT1: f64 = 0.425;
    const SPLIT2: f64 = 5.0;
    const A: [f64; 8] = [
        3.3871328727963666080e+00,
        1.3314166789178437745e+02,
        1.9715909503065514427e+03,
        1.3731693765509461125e+04,
        4.5921953931549871457e+04,
        6.7265770927008700853e+04,
        3.3430575583588128105e+04,
        2.5090809287301226727e+03,
    ];
    const B: [f64; 8] = [
        1.0000000000000000000e+00,
        4.2313330701600911252e+01,
        6.8718700749205790830e+02,
        5.3941960214247511077e+03,
        2.1213794301586595867e+04,
        3.9307895800092710610e+04,
        2.8729085735721942674e+04,
        5.2264952788528545610e+03,
    ];
    const C: [f64; 8] = [
        1.42343711074968357734e+00,
        4.63033784615654529590e+00,
        5.76949722146069140550e+00,
        3.64784832476320460504e+00,
        1.27045825245236838258e+00,
        2.41780725177450611770e-01,
        2.27238449892691845833e-02,
        7.74545014278341407640e-04,
    ];
    const D: [f64; 8] = [
        1.00000000000000000000e+00,
        2.05319162663775882187e+00,
        1.67638483018380384940e+00,
        6.89767334985100004550e-01,
        1.48103976427480074590e-01,
        1.51986665636164571966e-02,
        5.47593808499534494600e-04,
        1.05075007164441684324e-09,
    ];
    const E: [f64; 8] = [
        6.65790464350110377720e+00,
        5.46378491116411436990e+00,
        1.78482653991729133580e+00,
        2.96560571828504891230e-01,
        2.65321895265761230930e-02,
        1.24266094738807843860e-03,
        2.71155556874348757815e-05,
        2.01033439929228813265e-07,
    ];
    const F: [f64; 8] = [
        1.00000000000000000000e+00,
        5.99832206555887937690e-01,
        1.36929880922735805310e-01,
        1.48753612908506148525e-02,
        7.86869131145613259100e-04,
        1.84631831751005468180e-05,
        1.42151175831644588870e-07,
        2.04426310338993978564e-15,
    ];

    #[inline(always)]
    #[rustfmt::skip]
    fn poly(c: &[f64], x: f64) -> f64 {
        c[0] + x * (c[1] + x * (c[2] + x * (c[3] + x * (
        c[4] + x * (c[5] + x * (c[6] + x * (c[7])))))))
    }

    if p <= 0.0 {
        return f64::NEG_INFINITY;
    } else if 1.0 <= p {
        return f64::INFINITY;
    }

    let q = p - 0.5;

    if (if q < 0.0 { -q } else { q }) <= SPLIT1 {
        let x = CONST1 - q * q;
        return q * poly(&A, x) / poly(&B, x);
    }

    let mut x = if q < 0.0 { p } else { 1.0 - p };

    x = (-x.ln()).sqrt();

    if x <= SPLIT2 {
        x -= CONST2;
        x = poly(&C, x) / poly(&D, x);
    } else {
        x -= SPLIT2;
        x = poly(&E, x) / poly(&F, x);
    }

    if q < 0.0 { -x } else { x }
}

fn normal(x: f64) -> f64 {
    let b1 = 0.319381530f64;
    let b2 = -0.356563782f64;
    let b3 = 1.781477937f64;
    let b4 = -1.821255978f64;
    let b5 = 1.330274429f64;
    let p = 0.2316419f64;
    let c_0 = 0.39894228f64;
    let t = 1.0f64 / (1.0f64 + p * (if x < 0.0f64 { -x } else { x }));
    let series =
        1.0f64 - c_0 * (-x * x / 2.0f64).exp() * t * (t * (t * (t * (t * b5 + b4) + b3) + b2) + b1);
    if x > 0.0f64 { 1.0f64 - series } else { series }
}

struct Rnd;
/*
 * @brief Bindings for interacting with the random number generator.
 *
 * This module not only allows basic random number generation, but it also
 *  handles more complicated statistical stuff.
 *
 * Example usage would be:
 * @code
 * if rnd.rnd() < 0.5 then
 *    -- 50% chance of this happening
 * else
 *    -- And 50% chance of this happening
 * end
 * @endcode
 *
 * @luamod rnd
 */
impl UserData for Rnd {
    fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
        /*
         * @brief Gets a random number.  With no parameters it returns a random float
         * between 0 and 1.
         *
         * With one parameter it returns a whole number between 0 and that number
         *  (both included).  With two parameters it returns a whole number between
         *  both parameters (both included).
         *
         * @usage n = rnd.rnd() -- Number in range [0:1].
         * @usage n = rnd.rnd(5) -- Integer in range [0:5].
         * @usage n = rnd.rnd(3,5) -- Integer in range [3,5].
         *
         *    @luatparam number x First parameter, read description for details.
         *    @luatparam number y Second parameter, read description for details.
         *    @luatreturn number A randomly generated number, read description for
         * details.
         * @luafunc rnd
         */
        methods.add_function(
            "rnd",
            |_, (l, h): (Option<i64>, Option<i64>)| -> mlua::Result<mlua::Number> {
                if let Some(l) = l {
                    Ok(if let Some(h) = h {
                        if h < l { range(h..=l) } else { range(l..=h) }
                    } else {
                        range(0..=l)
                    } as mlua::Number)
                } else {
                    Ok(rng::<f64>())
                }
            },
        );
        /*
         * @brief Creates a number in the one-sigma range [-1:1].
         *
         * A one sigma range means that it creates a number following the normal
         * distribution but limited to the 63% quadrant.  This means that the number is
         * biassed towards 0, but can become either 1 or -1.  It's a fancier way of
         * generating random numbers.
         *
         * @usage n = 5.5 + rnd.sigma()/2. -- Creates a number from 5 to 6 slightly
         * biassed to 5.5.
         *    @luatreturn number A number from [-1:1] biassed slightly towards 0.
         * @luafunc sigma
         */
        methods.add_function("sigma", |_, ()| -> mlua::Result<mlua::Number> {
            Ok(normal_inverse(
                0.158655255 + rng::<f64>() * (1. - 0.158655255 * 2.),
            ))
        });
        /*
         * @brief Creates a number in the two-sigma range [-2:2].
         *
         * This function behaves much like the `rnd.sigma` function but uses the
         * two-sigma range, meaning that numbers are in the 95% quadrant and thus are
         * much more random.  They are biassed towards 0 and approximately 63% will be
         * within
         * [-1:1].  The rest will be in either the [-2:-1] range or the [1:2] range.
         *
         * @usage n = 5.5 + rnd.twosigma()/4. -- Creates a number from 5 to 6 heavily
         * biassed to 5.5.
         *
         *    @luatreturn number A number from [-2:2] biassed heavily towards 0.
         * @luafunc twosigma
         */
        methods.add_function("twosigma", |_, ()| -> mlua::Result<mlua::Number> {
            Ok(normal_inverse(
                0.022750132 + rng::<f64>() * (1. - 0.022750132 * 2.),
            ))
        });
        /*
         * @brief Creates a number in the three-sigma range [-3:3].
         *
         * This function behaves much like its sisters `rnd.sigma` and `rnd.twosigma`.
         * The main difference is that it uses the three-sigma range which is the 99%
         * quadrant.  It will rarely generate numbers outside the [-2:2] range (about 5%
         * of the time) and create numbers outside of the [-1:1] range about 37% of the
         * time.  This can be used when you want extremes to appear rarely.
         *
         * @usage n = 5.5 + rnd.threesigma()/6. -- Creates a number from 5 to 6 totally
         * biassed to 5.5.
         *
         *    @luatreturn number A number from [-3:3] biassed totally towards 0.
         * @luafunc threesigma
         */
        methods.add_function("threesigma", |_, ()| -> mlua::Result<mlua::Number> {
            Ok(normal_inverse(
                0.0013498985 + rng::<f64>() * (1. - 0.0013498985 * 2.),
            ))
        });
        /*
         * @brief Gets a random number in the given range, with a uniform distribution.
         *
         * @usage n = uniform() -- Real number in the interval [0,1).
         * @usage n = uniform(5) -- Real number in the interval [0,5).
         * @usage n = uniform(3,5) -- Real number in the interval [3,5).
         *
         *    @luatparam number x First parameter, read description for details.
         *    @luatparam number y Second parameter, read description for details.
         *    @luatreturn number A randomly generated number, read description for
         * details.
         * @luafunc uniform
         */
        methods.add_function(
            "uniform",
            |_, (l, h): (f64, f64)| -> mlua::Result<mlua::Number> { Ok(range(l..h)) },
        );
        /*
         * @brief Gets a random angle, i.e., a random number from 0 to 2*pi.
         *
         * @usage vec2.newP(radius, rnd.angle())
         *    @luatreturn number A randomly generated angle, in radians.
         * @luafunc angle
         */
        methods.add_function("angle", |_, ()| -> mlua::Result<mlua::Number> {
            Ok(range(0. ..2. * std::f64::consts::PI))
        });
        /*
         * @brief Creates a random permutation
         *
         * This creates a list from 1 to input and then randomly permutes it,
         * however, if an ordered table is passed as a parameter, that is randomly
         * permuted instead.
         *
         * @usage t = rnd.permutation( 5 )
         * @usage t = rnd.permutation( {"cat", "dog", "cheese"} )
         *
         *    @luatparam number|table input Maximum value to permute to.
         *    @luatreturn table A randomly permuted table.
         * @luafunc permutation
         */
        methods.add_function(
            "permutation",
            |lua, val: Either<u32, mlua::Table>| -> mlua::Result<mlua::Table> {
                use rand::seq::SliceRandom;
                let len = match val {
                    Either::Left(v) => v as usize,
                    Either::Right(ref tbl) => tbl.len()? as usize,
                };
                let mut vals: Vec<usize> = (0..len).collect();
                RNG.with_borrow_mut(|x| vals.shuffle(x));

                let t = lua.create_table()?;
                match val {
                    Either::Left(_) => {
                        for k in vals {
                            t.raw_push(k + 1)?;
                        }
                    }
                    Either::Right(tbl) => {
                        for k in vals {
                            let v: mlua::Value = tbl.get(k + 1)?;
                            t.raw_push(v)?;
                        }
                    }
                }
                Ok(t)
            },
        );
    }
}

pub fn open_rnd(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData> {
    Ok(lua.create_proxy::<Rnd>()?)
}

#[test]
fn main() {
    let lua = mlua::Lua::new();
    let globals = lua.globals();
    globals.set("rnd", open_rnd(&lua).unwrap()).unwrap();
    lua.load(include_str!("rng_test.lua"))
        .set_name("mlua Rng test")
        .exec()
        .unwrap();
}
