use rand::Rng;
use std::os::raw::{c_double, c_uint};

#[unsafe(no_mangle)]
pub unsafe extern "C" fn randint() -> c_uint {
    RNG.with_borrow_mut(|x| x.random::<u32>())
}
#[unsafe(no_mangle)]
pub unsafe extern "C" fn randfp() -> c_double {
    RNG.with_borrow_mut(|x| x.random::<f64>())
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

pub fn rngf32() -> f32 {
    RNG.with_borrow_mut(|x| x.random::<f32>())
}

/*
pub fn range(l: i32, h: i32) -> i32 {
    RNG.with_borrow_mut(|x| x.gen_range(l..h))
}
*/

/* Taken from probability package. */
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
