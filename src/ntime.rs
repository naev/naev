use crate::gettext::gettext;
use formatx::formatx;
use std::collections::VecDeque;
use std::ffi::CString;
use std::os::raw::{c_char, c_double, c_int, c_ulong};
use std::sync::Mutex;

pub type NTimeC = i64;
#[derive(Clone, Copy, PartialEq, Eq, Default, Debug)]
pub struct NTime(i64);
struct NTimeInternal {
    time: NTime,
    remainder: f64,
}
impl<T: Into<i64>> std::ops::Add<T> for NTime {
    type Output = NTime;
    fn add(self, other: T) -> Self {
        NTime(self.0 + other.into())
    }
}
impl<T: Into<i64>> std::ops::AddAssign<T> for NTime {
    fn add_assign(&mut self, other: T) {
        self.0 += other.into()
    }
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
impl From<NTime> for i64 {
    fn from(t: NTime) -> i64 {
        t.0
    }
}
impl std::fmt::Display for NTime {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        let cycles = self.cycles();
        let periods = self.periods();
        let seconds = self.seconds();
        // TODO try to move 2 to variable decimal length, but not that important
        if cycles == 0 && periods == 0 {
            write!(
                f,
                "{}",
                formatx!(gettext("{:04d} s").to_string(), seconds).unwrap()
            )
        } else if cycles == 0 {
            write!(
                f,
                "{}",
                formatx!(
                    gettext("{p:.2f} s").to_string(),
                    p = periods as f64 + 0.0001 * seconds as f64
                )
                .unwrap()
            )
        } else {
            write!(
                f,
                "{}",
                formatx!(
                    gettext("UST {c}:{p:.2f}").to_string(),
                    c = cycles,
                    p = periods as f64 + 0.0001 * seconds as f64
                )
                .unwrap()
            )
        }
    }
}
impl NTime {
    pub fn new(scu: i32, stp: i32, stu: i32) -> NTime {
        let scu = scu as i64;
        let stp = stp as i64;
        let stu = stu as i64;
        NTime(scu * (5_000 * 10_000 * 1_000) + stp * (10_000 * 1_000) + stu * 1_000)
    }
    pub fn cycles(self) -> i32 {
        let t = self.0;
        (t / (5_000 * 10_000 * 1_000)).try_into().unwrap_or(-1)
    }
    pub fn periods(self) -> i32 {
        let t = self.0;
        (t / (10_000 * 1_000) % 5_000).try_into().unwrap_or(-1)
    }
    pub fn seconds(self) -> i32 {
        let t = self.0;
        (t / 1_000 % 10_000).try_into().unwrap_or(-1)
    }
    pub fn remainder(self) -> f64 {
        let t = self.0 as f64;
        t % 1_000.
    }
    pub fn to_seconds(self) -> f64 {
        let t = self.0 as f64;
        t / 1_000.
    }
}

static DEFERLIST: Mutex<VecDeque<NTime>> = Mutex::new(VecDeque::new());
static TIME: Mutex<NTimeInternal> = Mutex::new(NTimeInternal {
    time: NTime(0),
    remainder: 0.,
});
static ENABLED: Mutex<bool> = Mutex::new(true);

#[no_mangle]
pub extern "C" fn ntime_update(dt: c_double) {
    update(dt);
}
#[no_mangle]
pub extern "C" fn ntime_create(scu: c_int, stp: c_int, stu: c_int) -> NTimeC {
    NTime::new(scu, stp, stu).0
}
#[no_mangle]
pub unsafe extern "C" fn ntime_get() -> NTimeC {
    get().0
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
    *rem = nt.time.remainder() + nt.remainder;
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
    NTime(t).to_seconds()
}
#[no_mangle]
pub unsafe extern "C" fn ntime_getRemainder(t: NTimeC) -> c_double {
    NTime(t).remainder()
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
    naevc::strdup(str.as_mut_ptr())
}
#[no_mangle]
pub unsafe extern "C" fn ntime_prettyBuf(cstr: *mut c_char, max: c_int, t: NTimeC, d: c_int) {
    let nt = if t == 0 {
        TIME.lock().unwrap().time
    } else {
        NTime(t)
    };
    let cycles = nt.cycles();
    let periods = nt.periods();
    let seconds = nt.seconds();
    let max = max as c_ulong;
    if cycles == 0 && periods == 0 {
        let cmsg = CString::new(gettext("%04d s")).unwrap();
        naevc::snprintf(cstr, max, cmsg.as_ptr().cast(), seconds);
    } else if cycles == 0 || d == 0 {
        let cmsg = CString::new(gettext("%.*f p")).unwrap();
        naevc::snprintf(
            cstr,
            max,
            cmsg.as_ptr().cast(),
            d,
            periods as c_double + 0.0001 * seconds as c_double,
        );
    } else {
        let cmsg = CString::new(gettext("UST %d:%.*f")).unwrap();
        naevc::snprintf(
            cstr,
            max,
            cmsg.as_ptr().cast(),
            cycles,
            d,
            periods as c_double + 0.0001 * seconds as c_double,
        );
    };
}
#[no_mangle]
pub extern "C" fn ntime_set(t: NTimeC) {
    set(NTime(t));
}
#[no_mangle]
pub extern "C" fn ntime_setR(cycles: c_int, periods: c_int, seconds: c_int, rem: c_double) {
    set_remainder(NTime::new(cycles, periods, seconds), rem);
}
#[no_mangle]
pub extern "C" fn ntime_inc(tc: NTimeC) {
    inc(NTime(tc));
}
#[no_mangle]
pub extern "C" fn ntime_allowUpdate(enable: c_int) {
    allow_update(enable != 0);
}
#[no_mangle]
pub extern "C" fn ntime_incLagged(t: NTimeC) {
    inc_queue(NTime(t));
}
#[no_mangle]
pub extern "C" fn ntime_refresh() {
    refresh();
}

pub fn get() -> NTime {
    TIME.lock().unwrap().time
}

pub fn set(t: NTime) {
    let mut nt = TIME.lock().unwrap();
    nt.time = t;
    nt.remainder = 0.;
}

pub fn set_remainder(t: NTime, rem: f64) {
    let mut nt = TIME.lock().unwrap();
    nt.time = t;
    nt.time += NTime(rem.floor() as i64);
    nt.remainder %= 1.0;
}

pub fn update(dt: f64) {
    if !*ENABLED.lock().unwrap() {
        return;
    }
    let mut nt = TIME.lock().unwrap();
    let dtt = nt.remainder + dt * 30. * 1000.;
    let tu = dtt.floor();
    let inc = tu as i64;
    nt.remainder = dtt - tu;
    nt.time += NTime(inc);
    unsafe { naevc::hooks_updateDate(inc) };
}

pub fn allow_update(enable: bool) {
    *ENABLED.lock().unwrap() = enable;
}

pub fn inc(t: NTime) {
    TIME.lock().unwrap().time += t;
    unsafe {
        naevc::economy_update(t.into());
    }
    if t > NTime(0) {
        unsafe {
            naevc::hooks_updateDate(t.into());
        }
    }
}

pub fn inc_queue(t: NTime) {
    DEFERLIST.lock().unwrap().push_back(t);
}

pub fn refresh() {
    while let Some(t) = DEFERLIST.lock().unwrap().pop_front() {
        TIME.lock().unwrap().time += t;
        unsafe {
            naevc::economy_update(t.into());
        }
    }
}