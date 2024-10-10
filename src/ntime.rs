use crate::gettext::gettext;
use derive_more::{Add, AddAssign};
use std::collections::VecDeque;
use std::ffi::CString;
use std::os::raw::{c_char, c_double, c_int, c_ulong};
use std::sync::Mutex;

pub type NTimeC = i64;
#[derive(Clone, Copy, PartialEq, Eq, Add, AddAssign)]
pub struct NTime(i64);

struct NTimeInternal {
    time: NTime,
    remainder: f64,
}
impl Ord for NTime {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        self.0.cmp(&other.0)
    }
}
impl PartialOrd for NTime {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}
impl From<NTime> for u32 {
    fn from(t: NTime) -> u32 {
        t.0.try_into().unwrap()
    }
}
impl NTime {
    pub fn create(scu: i32, stp: i32, stu: i32) -> NTime {
        let scu = scu as i64;
        let stp = stp as i64;
        let stu = stu as i64;
        NTime(scu * (5_000 * 10_000 * 1_000) + stp * (10_000 * 1_000) + stu * 1_000)
    }
    pub fn cycles(self) -> i32 {
        let t = self.0;
        (t / (5000 * 10000 * 1000)).try_into().unwrap()
    }
    pub fn periods(self) -> i32 {
        let t = self.0;
        (t / (10000 * 1000) % 5000).try_into().unwrap()
    }
    pub fn seconds(self) -> i32 {
        let t = self.0;
        (t / 1000 % 10000).try_into().unwrap()
    }
}

static DEFERLIST: Mutex<VecDeque<NTime>> = Mutex::new(VecDeque::new());
static TIME: Mutex<NTimeInternal> = Mutex::new(NTimeInternal {
    time: NTime(0),
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
    let inc = tu as i64;
    nt.remainder = dtt - tu;
    nt.time += NTime(inc);
    unsafe { naevc::hooks_updateDate(inc) };
}
#[no_mangle]
pub extern "C" fn ntime_create(scu: c_int, stp: c_int, stu: c_int) -> NTimeC {
    NTime::create(scu, stp, stu).0
}
#[no_mangle]
pub unsafe extern "C" fn ntime_get() -> NTimeC {
    return TIME.lock().unwrap().time.0;
}
#[no_mangle]
pub unsafe extern "C" fn ntime_getR(
    cycles: *mut c_int,
    periods: *mut c_int,
    seconds: *mut c_int,
    rem: *mut c_double,
) {
    let nt = TIME.lock().unwrap();
    let t = nt.time;
    *cycles = t.cycles();
    *periods = t.periods();
    *seconds = t.seconds();
    *rem = ntime_getRemainder(nt.time.0) + nt.remainder;
}
#[no_mangle]
pub unsafe extern "C" fn ntime_getCycles(t: NTimeC) -> c_int {
    NTime(t).cycles()
}
#[no_mangle]
pub unsafe extern "C" fn ntime_getPeriods(t: NTimeC) -> c_int {
    NTime(t).periods()
}
#[no_mangle]
pub unsafe extern "C" fn ntime_getSeconds(t: NTimeC) -> c_int {
    NTime(t).seconds()
}
#[no_mangle]
pub unsafe extern "C" fn ntime_convertSeconds(t: NTimeC) -> c_double {
    t as c_double / 1000.0
}
#[no_mangle]
pub unsafe extern "C" fn ntime_getRemainder(t: NTimeC) -> c_double {
    t as c_double % 1000.
}
#[no_mangle]
pub unsafe extern "C" fn ntime_pretty(t: NTimeC, d: c_int) -> *mut c_char {
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
pub unsafe extern "C" fn ntime_prettyBuf(cstr: *mut c_char, max: c_int, t: NTimeC, d: c_int) {
    let nt: NTime;
    if t == 0 {
        nt = TIME.lock().unwrap().time;
    } else {
        nt = NTime(t);
    }
    let cycles = nt.cycles();
    let periods = nt.periods();
    let seconds = nt.seconds();
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
pub extern "C" fn ntime_set(t: NTimeC) {
    let mut nt = TIME.lock().unwrap();
    nt.time = NTime(t);
    nt.remainder = 0.;
}
#[no_mangle]
pub extern "C" fn ntime_setR(cycles: c_int, periods: c_int, seconds: c_int, rem: c_double) {
    let mut nt = TIME.lock().unwrap();
    nt.time = NTime::create(cycles, periods, seconds);
    nt.time = nt.time + NTime(rem.floor() as i64);
    nt.remainder %= 1.0;
}
#[no_mangle]
pub extern "C" fn ntime_inc(tc: NTimeC) {
    let t = NTime(tc);
    TIME.lock().unwrap().time += t;
    unsafe {
        naevc::economy_update(t.into());
    }
    if t > NTime(0) {
        unsafe {
            naevc::hooks_updateDate(tc);
        }
    }
}
#[no_mangle]
pub extern "C" fn ntime_allowUpdate(enable: c_int) {
    *ENABLED.lock().unwrap() = enable != 0;
}
#[no_mangle]
pub extern "C" fn ntime_incLagged(t: NTimeC) {
    DEFERLIST.lock().unwrap().push_back(NTime(t));
}
#[no_mangle]
pub extern "C" fn ntime_refresh() {
    loop {
        match DEFERLIST.lock().unwrap().pop_front() {
            Some(t) => {
                TIME.lock().unwrap().time += t;
                unsafe {
                    naevc::economy_update(t.into());
                }
            }
            _ => break,
        }
    }
}
