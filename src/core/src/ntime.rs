use anyhow::Result;
use formatx::formatx;
use gettext::gettext;
use mlua::{FromLua, Lua, MetaMethod, UserData, UserDataMethods, Value, ffi};
use nlog::warn_err;
use regex::Regex;
use serde::{Deserialize, Deserializer};
use std::collections::VecDeque;
use std::ffi::CString;
use std::os::raw::{c_char, c_double, c_int, c_ulong, c_void};
use std::sync::{LazyLock, Mutex, RwLock};

pub type NTimeC = i64;
#[derive(
    Clone,
    Copy,
    derive_more::Add,
    derive_more::AddAssign,
    derive_more::Sub,
    derive_more::SubAssign,
    PartialOrd,
    PartialEq,
    Eq,
    Debug,
    Default,
)]
pub struct NTime(i64);

impl<'de> Deserialize<'de> for NTime {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'de>,
    {
        let data = String::deserialize(deserializer)?;
        NTime::from_string(&data).map_err(serde::de::Error::custom)
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
        CONVERTER.from_string(input)
    }
    pub fn as_string(self) -> String {
        CONVERTER.to_string(self)
    }
}

struct Converter {
    to_string: Option<mlua::Function>,
    from_string: Option<mlua::Function>,
}
impl Converter {
    fn new() -> Self {
        match Self::try_new() {
            Ok(c) => c,
            Err(e) => {
                warn_err!(e);
                Converter {
                    to_string: None,
                    from_string: None,
                }
            }
        }
    }

    fn try_new() -> Result<Self> {
        let lua = mlua::Lua::new_with(mlua::StdLib::ALL_SAFE, Default::default())?;
        let chunk = lua.load(ndata::read("time.lua")?);
        chunk.call::<()>(())?;
        let globals = lua.globals();
        let to_string: mlua::Function = globals.get("to_string")?;
        let from_string: mlua::Function = globals.get("from_string")?;
        Ok(Converter {
            to_string: Some(to_string),
            from_string: Some(from_string),
        })
    }

    fn to_string_default(nt: NTime) -> String {
        let cycles = nt.cycles();
        let periods = nt.periods();
        let seconds = nt.seconds();
        // TODO try to move 2 to variable decimal length, but not that important
        if cycles == 0 && periods == 0 {
            formatx!(gettext("{:04} s"), seconds).unwrap()
        } else if cycles == 0 {
            formatx!(gettext("{p}.{s:04} p"), p = periods, s = seconds,).unwrap()
        } else {
            formatx!(
                gettext("UST {c}:{p:04}.{s:04}"),
                c = cycles,
                p = periods,
                s = seconds,
            )
            .unwrap()
        }
    }

    fn to_string(&self, nt: NTime) -> String {
        if let Some(to_string) = &self.to_string {
            match to_string.call(nt) {
                Ok(s) => s,
                Err(e) => {
                    warn_err!(e);
                    Self::to_string_default(nt)
                }
            }
        } else {
            Self::to_string_default(nt)
        }
    }

    fn from_string(&self, input: &str) -> Result<NTime> {
        if let Some(from_string) = &self.from_string {
            Ok(from_string.call(input)?)
        } else {
            static RE_UST: LazyLock<Regex> = LazyLock::new(|| {
                Regex::new(r"^\s*UST\s+(\d+)(?::(\d{4})(?:\.(\d{4}))?)?\s*$").unwrap()
            });
            static RE_P: LazyLock<Regex> =
                LazyLock::new(|| Regex::new(r"^\s*(\d+)(?:\.(\d{4}))?\s+p\s*$").unwrap());
            static RE_S: LazyLock<Regex> =
                LazyLock::new(|| Regex::new(r"^\s*(\d+)\s+s\s*$").unwrap());

            if let Some(cap) = RE_UST.captures(input) {
                return Ok(NTime::new(
                    cap[1].parse()?,
                    match cap.get(2) {
                        Some(m) => m.as_str().parse::<i32>()?,
                        None => 0,
                    },
                    match cap.get(3) {
                        Some(m) => m.as_str().parse::<i32>()?,
                        None => 0,
                    },
                ));
            }

            if let Some(cap) = RE_P.captures(input) {
                return Ok(NTime::new(
                    0,
                    cap[1].parse()?,
                    match cap.get(2) {
                        Some(m) => m.as_str().parse::<i32>()?,
                        None => 0,
                    },
                ));
            }

            if let Some(cap) = RE_S.captures(input) {
                return Ok(NTime::new(0, 0, cap[1].parse()?));
            }

            anyhow::bail!("not valid ntime")
        }
    }
}

static DEFERLIST: Mutex<VecDeque<NTime>> = Mutex::new(VecDeque::new());
static TIME: RwLock<NTimeInternal> = RwLock::new(NTimeInternal::new());
static ENABLED: Mutex<bool> = Mutex::new(true);
static CONVERTER: LazyLock<Converter> = LazyLock::new(|| Converter::new());

impl FromLua for NTime {
    fn from_lua(value: Value, _: &Lua) -> mlua::Result<Self> {
        match value {
            Value::UserData(ud) => Ok(*ud.borrow::<Self>()?),
            /*
            Value::Integer(num) => Ok(Self::new(num as f64, num as f64)),
            Value::Number(num) => Ok(Self::new(num, num)),
            Value::Table(tbl) => {
                let x: f64 = tbl.get(1)?;
                let y: f64 = tbl.get(2)?;
                Ok(Self::new(x, y))
            }
            */
            val => Err(mlua::Error::RuntimeError(format!(
                "unable to convert {} to NTime",
                val.type_name()
            ))),
        }
    }
}

/*@
 * @brief Bindings for interacting with the time.
 *
 * Usage is generally something as follows:
 * @code
 * time_limit = time.get() + time.new( 0, 5, 0 )
 * player.msg( string.format("You only have %s left!", time.str(time.get() -
 * time_limit)) )
 *
 * -- Do stuff here
 *
 * if time.get() > time_limit then
 *    -- Limit is up
 * end
 * @endcode
 *
 * @luamod time
 */
impl UserData for NTime {
    fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
        /*@
         * @brief Creates a time. This can be absolute or relative.
         *
         * @usage t = time.new( 591, 3271, 12801 ) -- Gets a time near when the incident
         * happened.
         *
         *    @luatparam number cycles Cycles for the new time.
         *    @luatparam number periods Periods for the new time.
         *    @luatparam number seconds Seconds for the new time.
         *    @luatreturn Time A newly created time metatable.
         * @luafunc new
         */
        methods.add_function(
            "new",
            |_, (scu, stp, stu): (i32, i32, i32)| -> mlua::Result<Self> {
                Ok(NTime::new(scu, stp, stu))
            },
        );
        /*@
         * @brief Gets the current time in internal representation time.
         *
         * @usage t = time.cur()
         *
         *    @luatreturn Time Time in internal representation time.
         * @luafunc cur
         */
        methods.add_function("cur", |_, ()| -> mlua::Result<Self> {
            Ok(TIME.read().unwrap().time)
        });
        methods.add_function("get", |lua, ()| -> mlua::Result<Self> {
            crate::lua::deprecated(lua, "get", Some("cur"))?;
            Ok(TIME.read().unwrap().time)
        });
        /*@
         * @brief Sets the current in-game time.
         *
         *    @luatparam Time Time to set to.
         * @luafunc set_current
         */
        methods.add_function("set_current", |_, nt: Self| -> mlua::Result<()> {
            TIME.write().unwrap().time = nt;
            Ok(())
        });
        /*@
         * @brief Adds two time metatables.
         *
         * Overrides the addition operator.
         *
         * @usage new_time = time.get() + time.new( 0, 5, 0 ) -- Adds 5 periods to the
         * current date
         * @usage t:add( time.new( 0, 3, 0 ) ) -- Directly modifies t
         *
         *    @luatparam Time t1 Time metatable to add to.
         *    @luatparam Time t2 Time metatable added.
         * @luafunc add
         */
        methods.add_meta_function(
            MetaMethod::Add,
            |_, (nt1, nt2): (Self, Self)| -> mlua::Result<Self> { Ok(nt1 + nt2) },
        );
        methods.add_method_mut("add", |_, nt1, nt2: Self| -> mlua::Result<Self> {
            *nt1 += nt2;
            Ok(*nt1)
        });
        /*@
         * @brief Subtracts two time metatables.
         *
         * Overrides the subtraction operator.
         *
         * @usage new_time = time.get() - time.new( 0, 3, 0 ) -- Subtracts 3 periods
         * from the current date
         * @usage t:sub( time.new( 0, 3, 0 ) ) -- Directly modifies t
         *
         *    @luatparam Time t1 Time metatable to subtract from.
         *    @luatparam Time t2 Time metatable subtracted.
         * @luafunc sub
         */
        methods.add_meta_function(MetaMethod::Sub, |_, (nt1, nt2): (Self, Self)| Ok(nt1 - nt2));
        methods.add_method_mut("sub", |_, nt1, nt2: Self| {
            *nt1 -= nt2;
            Ok(*nt1)
        });
        /*@
         * @brief Checks to see if two time are equal.
         *
         * It is recommended to check with < and <= instead of ==.
         *
         * @usage if time.new( 630, 5, 78) == time.get() then -- do something if they
         * match
         *
         *    @luatparam Time t1 Time to compare for equality.
         *    @luatparam Time t2 Time to compare for equality.
         *    @luatreturn boolean true if they're equal.
         * @luafunc __eq
         */
        methods.add_meta_function(MetaMethod::Eq, |_, (nt1, nt2): (Self, Self)| Ok(nt1 == nt2));
        /*@
         * @brief Checks to see if a time is larger or equal to another.
         *
         * @usage if time.new( 630, 5, 78) <= time.get() then -- do something if time is
         * past UST 630:0005.78
         *
         *    @luatparam Time t1 Time to see if is is smaller or equal to than t2.
         *    @luatparam Time t2 Time see if is larger or equal to than t1.
         *    @luatreturn boolean true if t1 <= t2
         * @luafunc __le
         */
        methods.add_meta_function(MetaMethod::Le, |_, (nt1, nt2): (Self, Self)| Ok(nt1 <= nt2));
        /*@
         * @brief Checks to see if a time is strictly larger than another.
         *
         * @usage if time.new( 630, 5, 78) < time.get() then -- do something if time is
         * past UST 630:0005.78
         *
         *    @luatparam Time t1 Time to see if is is smaller than t2.
         *    @luatparam Time t2 Time see if is larger than t1.
         *    @luatreturn boolean true if t1 < t2
         * @luafunc __lt
         */
        methods.add_meta_function(MetaMethod::Lt, |_, (nt1, nt2): (Self, Self)| Ok(nt1 < nt2));
        /*@
         * @brief Converts the time to a pretty human readable format.
         *
         * @usage strt = tostring( time.cur() ) -- Gets current time
         * @usage strt = tostring( time.get() + time.new(0,5,0) ) -- Gets time in 5
         * periods
         * @usage strt = t:__tostring() -- Gets the string of t
         *
         *    @luatparam Time t Time to convert to pretty format.  If omitted, current
         * time is used.
         *    @luatreturn string The time in human readable format.
         * @luafunc __tostring
         */
        methods.add_meta_function(
            MetaMethod::ToString,
            |_, nt: Self| -> mlua::Result<String> { Ok(nt.to_string()) },
        );
        /*@
         * @brief Increases or decreases the in-game time.
         *
         * Note that this can trigger hooks and fail missions and the likes.
         *
         * @usage time.inc( time.new(0,0,100) ) -- Increments the time by 100 seconds.
         *
         *    @luatparam Time t Amount to increment or decrement the time by.
         * @luafunc inc
         */
        methods.add_method("inc", |_, nt, ()| -> mlua::Result<()> {
            inc(*nt);
            Ok(())
        });
        /*@
         * @brief Gets a number representing this time.
         *
         * The best usage for this currently is mission variables.
         *
         * @usage num = t:tonumber() -- Getting the number from a time t
         *
         *    @luatparam Time t Time to get number of.
         *    @luatreturn number Number representing time.
         * @luafunc tonumber
         */
        methods.add_method("tonumber", |_, nt, ()| -> mlua::Result<i64> { Ok(nt.0) });
        /*@
         * @brief Creates a time from a number representing it.
         *
         * The best usage for this currently is mission variables.
         *
         * @usage t = time.fromnumber( t:tonumber() ) -- Should get the time t again
         *
         *    @luatparam number num Number to get time from.
         *    @luatreturn Time Time representing number.
         * @luafunc fromnumber
         */
        methods.add_function("fromnumber", |_, n: i64| -> mlua::Result<Self> {
            Ok(NTime(n))
        });
    }
}

pub fn open_time(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData> {
    let proxy = lua.create_proxy::<NTime>()?;

    // Only add stuff as necessary
    if let mlua::Value::Nil = lua.named_registry_value("push_time")? {
        let push_time = lua.create_function(|lua, n: i64| {
            let nt = NTime(n);
            lua.create_any_userdata(nt)
        })?;
        lua.set_named_registry_value("push_time", push_time)?;

        let get_time = lua.create_function(|_, mut ud: mlua::UserDataRefMut<NTime>| {
            let vec: *mut NTime = &mut *ud;
            Ok(Value::LightUserData(mlua::LightUserData(
                vec as *mut c_void,
            )))
        })?;
        lua.set_named_registry_value("get_time", get_time)?;
    }

    Ok(proxy)
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn luaL_checktime(L: *mut mlua::lua_State, idx: c_int) -> *mut NTime {
    unsafe {
        let vec = lua_totime(L, idx);
        if vec.is_null() {
            ffi::luaL_typerror(L, idx, c"time".as_ptr() as *const c_char);
        }
        vec
    }
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn lua_istime(L: *mut mlua::lua_State, idx: c_int) -> c_int {
    !lua_totime(L, idx).is_null() as c_int
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn lua_pushtime(L: *mut mlua::lua_State, nt: naevc::ntime_t) {
    unsafe {
        ffi::lua_getfield(L, ffi::LUA_REGISTRYINDEX, c"push_time".as_ptr());
        ffi::lua_pushinteger(L, nt);
        ffi::lua_call(L, 1, 1);
    }
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn lua_totime(L: *mut mlua::lua_State, idx: c_int) -> *mut NTime {
    unsafe {
        let idx = ffi::lua_absindex(L, idx);
        ffi::lua_getfield(L, ffi::LUA_REGISTRYINDEX, c"get_time".as_ptr());
        ffi::lua_pushvalue(L, idx);
        let vec = match ffi::lua_pcall(L, 1, 1, 0) {
            ffi::LUA_OK => ffi::lua_touserdata(L, -1) as *mut NTime,
            _ => std::ptr::null_mut(),
        };
        ffi::lua_pop(L, 1);
        vec
    }
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn luaL_validtime(L: *mut mlua::lua_State, idx: c_int) -> naevc::ntime_t {
    unsafe { (*luaL_checktime(L, idx)).0 }
}

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
fn test_ntime_ust() {
    assert!(NTime::from_string("cat").is_err());
    assert!(NTime::from_string("UST 123:cat.4567").is_err());
    assert!(NTime::from_string("UST 123:4567.dog").is_err());
    assert!(NTime::from_string("cat UST 603:3726.2871").is_err());
    assert!(NTime::from_string("UST 603:3726.2871 cat").is_err());
    assert!(NTime::from_string("UST 603:3726.n2871").is_err());
    assert!(NTime::from_string("UST 123:123:4567").is_err());
    assert_eq!(
        NTime::from_string("  UST   123:0001.9999  ").unwrap(),
        NTime::new(123, 1, 9999)
    );
    assert!(NTime::from_string("UST 123:456.8901").is_err());
    assert!(NTime::from_string("UST 123:4567.89").is_err());
    assert!(NTime::from_string("UST 123.8901").is_err());
    assert!(NTime::from_string("UST 123:0123:4567").is_err());
    assert_eq!(
        NTime::from_string("UST 123:0123.4567").unwrap(),
        NTime::new(123, 123, 4567)
    );
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

#[test]
fn test_ntime_p() {
    assert!(NTime::from_string("123.456 p").is_err());
    assert!(NTime::from_string("123.0456 p cat").is_err());
    assert!(NTime::from_string("cat 123.0456 p").is_err());
    assert!(NTime::from_string("123.0456 pp").is_err());
    assert_eq!(
        NTime::from_string("123.4567 p").unwrap(),
        NTime::new(0, 123, 4567)
    );
    assert_eq!(NTime::from_string("123 p").unwrap(), NTime::new(0, 123, 0));
}

#[test]
fn test_ntime_s() {
    assert!(NTime::from_string("123.45 s").is_err());
    assert!(NTime::from_string("123:45 s").is_err());
    assert_eq!(NTime::from_string("123 s").unwrap(), NTime::new(0, 0, 123));
}

// Fails to compile for now, so disabled
/*
#[test]
fn test_mlua_time() {
    let lua = mlua::Lua::new();
    let globals = lua.globals();
    globals.set("time", open_time(&lua).unwrap()).unwrap();
    lua.load(include_str!("ntime_test.lua"))
        .set_name("mlua NTime test")
        .exec()
        .unwrap();
}
*/
