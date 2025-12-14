use anyhow::{Context, Result};
use formatx::formatx;
use gettext::gettext;
use regex::Regex;
use serde::{Deserialize, Deserializer};
use std::collections::VecDeque;
use std::ffi::CString;
use std::os::raw::{c_char, c_double, c_int, c_ulong};
use std::sync::{Mutex, RwLock};

pub type NTimeC = i64;
#[derive(Clone, Copy, PartialEq, Eq, Debug, Default)]
pub struct NTime(i64);

impl<'de> Deserialize<'de> for NTime {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'de>,
    {
        let data = String::deserialize(deserializer)?;
        Ok(NTime::from_string(&data).unwrap())
    }
}
struct NTimeInternal {
    time: NTime,
    remainder: f64,
}
impl NTimeInternal {
    pub const fn new() -> Self {
        Self {
            time: NTime(0),
            remainder: 0.,
        }
    }
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
        write!(f, "{}", self.as_string())
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
        (t / (5_000 * 10_000 * 1_000)).try_into().unwrap_or(0)
    }
    pub fn periods(self) -> i32 {
        let t = self.0;
        (t / (10_000 * 1_000) % 5_000).try_into().unwrap_or(0)
    }
    pub fn seconds(self) -> i32 {
        let t = self.0;
        (t / 1_000 % 10_000).try_into().unwrap_or(0)
    }
    pub fn remainder(self) -> f64 {
        let t = self.0 as f64;
        t % 1_000.
    }
    pub fn to_seconds(self) -> f64 {
        let t = self.0 as f64;
        t / 1_000.
    }
    pub fn from_string(input: &str) -> Result<Self> {
        fn parse(cap: regex::Captures) -> Result<NTime> {
            let scu: i32 = cap[1].parse()?;
            let stp = match cap.get(2) {
                Some(m) => m.as_str().parse::<i32>()?,
                None => 0,
            };
            let stu = match cap.get(3) {
                Some(m) => m.as_str().parse::<i32>()?,
                None => 0,
            };
            Ok(NTime::new(scu, stp, stu))
        }
        let re = Regex::new(r"^\s*UST\s+(\d+)(?::(\d+)(?:\.(\d+))?)?\s*$").unwrap();

        let dates = re
            .captures_iter(input)
            .map(|cap| parse(cap))
            .collect::<Result<Vec<_>>>()?;
        dates.into_iter().next().context("not valid date")
    }
    pub fn as_string(self) -> String {
        let cycles = self.cycles();
        let periods = self.periods();
        let seconds = self.seconds();
        // TODO try to move 2 to variable decimal length, but not that important
        if cycles == 0 && periods == 0 {
            formatx!(gettext("{:04} s").to_string(), seconds).unwrap()
        } else if cycles == 0 {
            formatx!(
                gettext("{p:.2} s").to_string(),
                p = periods as f64 + 0.0001 * seconds as f64
            )
            .unwrap()
        } else {
            formatx!(
                gettext("UST {c}:{p:04.2}").to_string(),
                c = cycles,
                p = periods as f64 + 0.0001 * seconds as f64
            )
            .unwrap()
        }
    }
}

static DEFERLIST: Mutex<VecDeque<NTime>> = Mutex::new(VecDeque::new());
static TIME: RwLock<NTimeInternal> = RwLock::new(NTimeInternal::new());
static ENABLED: Mutex<bool> = Mutex::new(true);

#[unsafe(no_mangle)]
pub extern "C" fn ntime_update(dt: c_double) {
    update(dt);
}
#[unsafe(no_mangle)]
pub extern "C" fn ntime_create(scu: c_int, stp: c_int, stu: c_int) -> NTimeC {
    NTime::new(scu, stp, stu).0
}
#[unsafe(no_mangle)]
pub extern "C" fn ntime_get() -> NTimeC {
    get().0
}
#[unsafe(no_mangle)]
pub extern "C" fn ntime_getR(
    cycles: *mut c_int,
    periods: *mut c_int,
    seconds: *mut c_int,
    rem: *mut c_double,
) {
    let nt = TIME.read().unwrap();
    let t = nt.time;
    unsafe {
        *cycles = t.cycles();
        *periods = t.periods();
        *seconds = t.seconds();
        *rem = nt.time.remainder() + nt.remainder;
    }
}
#[unsafe(no_mangle)]
pub extern "C" fn ntime_getCycles(t: NTimeC) -> c_int {
    NTime(t).cycles()
}
#[unsafe(no_mangle)]
pub extern "C" fn ntime_getPeriods(t: NTimeC) -> c_int {
    NTime(t).periods()
}
#[unsafe(no_mangle)]
pub extern "C" fn ntime_getSeconds(t: NTimeC) -> c_int {
    NTime(t).seconds()
}
#[unsafe(no_mangle)]
pub extern "C" fn ntime_convertSeconds(t: NTimeC) -> c_double {
    NTime(t).to_seconds()
}
#[unsafe(no_mangle)]
pub extern "C" fn ntime_getRemainder(t: NTimeC) -> c_double {
    NTime(t).remainder()
}
#[unsafe(no_mangle)]
pub extern "C" fn ntime_pretty(t: NTimeC, d: c_int) -> *mut c_char {
    let mut str: [c_char; 64] = [0; 64];
    unsafe {
        ntime_prettyBuf(
            str.as_mut_ptr(),
            ::core::mem::size_of::<[c_char; 64]>() as c_ulong as c_int,
            t,
            d,
        );
        naevc::strdup(str.as_mut_ptr())
    }
}
#[unsafe(no_mangle)]
pub extern "C" fn ntime_prettyBuf(cstr: *mut c_char, max: c_int, t: NTimeC, d: c_int) {
    let nt = if t == 0 {
        TIME.read().unwrap().time
    } else {
        NTime(t)
    };
    let cycles = nt.cycles();
    let periods = nt.periods();
    let seconds = nt.seconds();
    let max = max as usize;
    if cycles == 0 && periods == 0 {
        let cmsg = CString::new(gettext("%04d s")).unwrap();
        unsafe {
            naevc::scnprintf(cstr, max, cmsg.as_ptr().cast(), seconds);
        }
    } else if cycles == 0 || d == 0 {
        let cmsg = CString::new(gettext("%.*f p")).unwrap();
        unsafe {
            naevc::scnprintf(
                cstr,
                max,
                cmsg.as_ptr().cast(),
                d,
                periods as c_double + 0.0001 * seconds as c_double,
            );
        }
    } else {
        let cmsg = CString::new(gettext("UST %d:%.*f")).unwrap();
        unsafe {
            naevc::scnprintf(
                cstr,
                max,
                cmsg.as_ptr().cast(),
                cycles,
                d,
                periods as c_double + 0.0001 * seconds as c_double,
            );
        }
    };
}
#[unsafe(no_mangle)]
pub extern "C" fn ntime_set(t: NTimeC) {
    set(NTime(t));
}
#[unsafe(no_mangle)]
pub extern "C" fn ntime_setR(cycles: c_int, periods: c_int, seconds: c_int, rem: c_double) {
    set_remainder(NTime::new(cycles, periods, seconds), rem);
}
#[unsafe(no_mangle)]
pub extern "C" fn ntime_inc(tc: NTimeC) {
    inc(NTime(tc));
}
#[unsafe(no_mangle)]
pub extern "C" fn ntime_allowUpdate(enable: c_int) {
    allow_update(enable != 0);
}
#[unsafe(no_mangle)]
pub extern "C" fn ntime_incLagged(t: NTimeC) {
    inc_queue(NTime(t));
}
#[unsafe(no_mangle)]
pub extern "C" fn ntime_refresh() {
    refresh();
}

pub fn get() -> NTime {
    TIME.read().unwrap().time
}

pub fn set(t: NTime) {
    let mut nt = TIME.write().unwrap();
    nt.time = t;
    nt.remainder = 0.;
}

pub fn set_remainder(t: NTime, rem: f64) {
    let mut nt = TIME.write().unwrap();
    nt.time = t;
    nt.time += NTime(rem.floor() as i64);
    nt.remainder %= 1.0;
}

pub fn update(dt: f64) {
    if !*ENABLED.lock().unwrap() {
        return;
    }
    let mut nt = TIME.write().unwrap();
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
    TIME.write().unwrap().time += t;
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
        TIME.write().unwrap().time += t;
        unsafe {
            naevc::economy_update(t.into());
        }
    }
}

#[test]
fn test_ntime() {
    assert!(NTime::from_string("cat").is_err());
    assert!(NTime::from_string("UST 123:cat.4567").is_err());
    assert!(NTime::from_string("UST 123:4567.dog").is_err());
    assert!(NTime::from_string("cat UST 603:3726.2871").is_err());
    assert!(NTime::from_string("UST 603:3726.2871 cat").is_err());
    assert!(NTime::from_string("UST 603:3726.n2871").is_err());
    assert_eq!(
        NTime::from_string("UST 603:3726.2871").unwrap(),
        NTime::new(603, 3726, 2871)
    );
    assert_eq!(
        NTime::from_string("UST 603:3726").unwrap(),
        NTime::new(603, 3726, 0)
    );
    assert_eq!(
        NTime::from_string("UST 603").unwrap(),
        NTime::new(603, 0, 0)
    );
}
