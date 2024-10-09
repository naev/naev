use crate::gettext::gettext;
use std::collections::VecDeque;
use std::ffi::CString;
use std::os::raw::{c_char, c_double, c_int, c_uint, c_ulong};
use std::sync::Mutex;

pub type NTime = i64;

struct NTimeInternal {
    time: NTime,
    remainder: f64,
}

static DEFERLIST: Mutex<VecDeque<NTime>> = Mutex::new(VecDeque::new());
static TIME: Mutex<NTimeInternal> = Mutex::new(NTimeInternal {
    time: 0,
    remainder: 0.,
});
static ENABLED: Mutex<bool> = Mutex::new(true);

#[no_mangle]
pub extern "C" fn ntime_update(cdt: c_double) {
    if !*ENABLED.lock().unwrap() {
        return;
    }
    let mut nt = TIME.lock().unwrap();
    let dt = cdt as f64;
    let dtt = nt.remainder + dt * 30. * 1000.;
    let tu = dtt.floor();
    let inc = tu as NTime;
    nt.remainder = dtt - tu;
    nt.time += inc;
    unsafe { naevc::hooks_updateDate(inc) };
}
#[no_mangle]
pub extern "C" fn ntime_create(cscu: c_int, cstp: c_int, cstu: c_int) -> NTime {
    let scu = cscu as NTime;
    let stp = cstp as NTime;
    let stu = cstu as NTime;
    return scu * (5_000 * 10_000 * 1_000) + stp * (10_000 * 1_000) + stu * 1_000;
}
#[no_mangle]
pub unsafe extern "C" fn ntime_get() -> NTime {
    return TIME.lock().unwrap().time;
}
#[no_mangle]
pub unsafe extern "C" fn ntime_getR(
    cycles: *mut c_int,
    periods: *mut c_int,
    seconds: *mut c_int,
    rem: *mut c_double,
) {
    let nt = TIME.lock().unwrap();
    *cycles = ntime_getCycles(nt.time);
    *periods = ntime_getPeriods(nt.time);
    *seconds = ntime_getSeconds(nt.time);
    *rem = ntime_getRemainder(nt.time) + nt.remainder;
}
#[no_mangle]
pub unsafe extern "C" fn ntime_getCycles(t: NTime) -> c_int {
    return (t / (5000 as NTime * 10000 as NTime * 1000 as NTime)) as c_int;
}
#[no_mangle]
pub unsafe extern "C" fn ntime_getPeriods(t: NTime) -> c_int {
    return (t / (10000 as NTime * 1000 as NTime) % 5000 as NTime) as c_int;
}
#[no_mangle]
pub unsafe extern "C" fn ntime_getSeconds(t: NTime) -> c_int {
    return (t / 1000 as NTime % 10000 as NTime) as c_int;
}
#[no_mangle]
pub unsafe extern "C" fn ntime_convertSeconds(t: NTime) -> c_double {
    return t as c_double / 1000 as c_int as c_double;
}
#[no_mangle]
pub unsafe extern "C" fn ntime_getRemainder(t: NTime) -> c_double {
    return (t % 1000 as NTime) as c_double;
}
#[no_mangle]
pub unsafe extern "C" fn ntime_pretty(t: NTime, d: c_int) -> *mut c_char {
    let mut str: [c_char; 64] = [0; 64];
    ntime_prettyBuf(
        str.as_mut_ptr(),
        ::core::mem::size_of::<[c_char; 64]>() as c_ulong as c_int,
        t,
        d,
    );
    return naevc::strdup(str.as_mut_ptr());
}
#[no_mangle]
pub unsafe extern "C" fn ntime_prettyBuf(cstr: *mut c_char, max: c_int, t: NTime, d: c_int) {
    let nt: NTime;
    if t == 0 {
        nt = TIME.lock().unwrap().time;
    } else {
        nt = t;
    }
    let cycles = ntime_getCycles(nt);
    let periods = ntime_getPeriods(nt);
    let seconds = ntime_getSeconds(nt);
    let max = max as c_ulong;
    if cycles == 0 && periods == 0 {
        let cmsg = CString::new(gettext("%04d s")).unwrap();
        naevc::snprintf(cstr, max, cmsg.as_ptr().cast(), seconds);
    } else if cycles == 0 as c_int || d == 0 as c_int {
        let cmsg = CString::new(gettext("%.*f p")).unwrap();
        naevc::snprintf(
            cstr,
            max,
            cmsg.as_ptr().cast(),
            d,
            periods as c_double + 0.0001f64 * seconds as c_double,
        );
    } else {
        let cmsg = CString::new(gettext("UST %d:%.*f")).unwrap();
        naevc::snprintf(
            cstr,
            max,
            cmsg.as_ptr().cast(),
            cycles,
            d,
            periods as c_double + 0.0001f64 * seconds as c_double,
        );
    };
}
#[no_mangle]
pub extern "C" fn ntime_set(t: NTime) {
    let mut nt = TIME.lock().unwrap();
    nt.time = t;
    nt.remainder = 0.;
}
#[no_mangle]
pub extern "C" fn ntime_setR(cycles: c_int, periods: c_int, seconds: c_int, rem: c_double) {
    let mut nt = TIME.lock().unwrap();
    nt.time = ntime_create(cycles, periods, seconds);
    nt.time = nt.time + (rem.floor() as NTime);
    nt.remainder %= 1.0;
}
#[no_mangle]
pub extern "C" fn ntime_inc(t: NTime) {
    TIME.lock().unwrap().time += t;
    unsafe {
        naevc::economy_update(t as c_uint);
    }
    if t > 0 as NTime {
        unsafe {
            naevc::hooks_updateDate(t);
        }
    }
}
#[no_mangle]
pub extern "C" fn ntime_allowUpdate(enable: c_int) {
    *ENABLED.lock().unwrap() = enable != 0;
}
#[no_mangle]
pub extern "C" fn ntime_incLagged(t: NTime) {
    DEFERLIST.lock().unwrap().push_back(t);
}
#[no_mangle]
pub extern "C" fn ntime_refresh() {
    loop {
        match DEFERLIST.lock().unwrap().pop_front() {
            Some(t) => {
                TIME.lock().unwrap().time += t;
                unsafe {
                    naevc::economy_update(t as c_uint);
                }
            }
            _ => break,
        }
    }
}
