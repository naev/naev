#![allow(clippy::not_unsafe_ptr_arg_deref)]
pub use anyhow;
use anyhow::{Error, Result};
use formatx::formatx;
use log::{debug, debugx, info, infox, warn, warn_err, warnx};
use ndata::env;
use sdl3 as sdl;
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int, c_uint, c_void}; // Re-export for outter rust shenanigans

#[link(name = "naev")]
unsafe extern "C" {
    /// Main function in C
    pub fn naev_main() -> c_int;
}

mod array;
mod constants;
mod damagetype;
mod faction;
mod input;
mod linebreak;
mod model;
mod nebula;
mod nlua;
mod outfit;
mod rng;
mod ship;
mod slots;
mod lua {
    pub mod ryaml;
}

use gettext::gettext;

use std::sync::atomic::AtomicBool;
static _QUIT: AtomicBool = AtomicBool::new(false);

/// Small wrapper to convert a C char* pointer to CStr
unsafe fn cptr_to_cstr<'a>(s: *const c_char) -> &'a str {
    unsafe { CStr::from_ptr(s).to_str().unwrap() }
}

/// Restarts the process, *should* be cross-platform
pub fn restart() -> Result<()> {
    use std::env;
    match cargo_util::ProcessBuilder::new(env::current_exe()?)
        .args(&env::args_os().skip(1).collect::<Vec<_>>())
        .exec_replace()
    {
        Ok(_) => std::process::exit(0),
        Err(e) => {
            std::process::exit(e.downcast::<cargo_util::ProcessError>()?.code.unwrap_or(1));
        }
    }
}
#[unsafe(no_mangle)]
pub extern "C" fn naev_restart() -> c_int {
    match restart() {
        Ok(()) => 0,
        Err(e) => {
            warn_err!(e);
            1
        }
    }
}

/// Entry Point
pub fn naev() -> Result<()> {
    match naevmain() {
        Ok(_) => Ok(()),
        Err(e) => {
            sdl::messagebox::show_simple_message_box(
                sdl::messagebox::MessageBoxFlag::ERROR,
                gettext("Naev Critical Error"),
                &format!(
                    "{}:\n{e}",
                    gettext("Naev Failed to start due to a critical error")
                ),
                None,
            )?;
            Err(e)
        }
    }
}

fn naevmain() -> Result<()> {
    /* Load up the argv and argc for the C main. */
    let args: Vec<String> = std::env::args().collect();
    let mut cargs = vec![];
    for a in args {
        cargs.push(CString::new(a).unwrap())
    }
    let mut argv = cargs.into_iter().map(|s| s.into_raw()).collect::<Vec<_>>();
    argv.shrink_to_fit();

    /* Begin logging infrastructure. */
    log::init().unwrap_or_else(|e| {
        warn_err!(e);
    });

    // Workarounds
    if cfg!(target_os = "linux") {
        // Set AMD_DEBUG environment variable before initializing OpenGL to
        // workaround driver bug. TODO remove around 0.14.0 or when fixed (maybe changing
        // backend?).
        unsafe {
            std::env::set_var("AMD_DEBUG", "nooptvariant");
        }
    }

    /* Start up PHYSFS. */
    unsafe {
        let argv0 = CString::new(env::ENV.argv0.clone()).unwrap();
        if !naevc::SDL_PhysFS_Init(argv0.as_ptr() as *const c_char) {
            let err = ndata::physfs::error_as_io_error("SDL_PhysFS_init");
            return Err(Error::new(err));
        }
        naevc::PHYSFS_permitSymbolicLinks(1);
    }

    /* Set up locales. */
    linebreak::init();
    gettext::init();

    /* Print the version */
    info!("{}", &*log::version::VERSION_HUMAN);
    if cfg!(target_os = "linux") {
        match env::ENV.is_appimage {
            true => {
                info!("AppImage detected. Running from: {}", &env::ENV.appdir)
            }
            false => debug!("AppImage not detected."),
        }
    }

    /* Initialize SDL. */
    let sdlctx = sdl::init()?;

    let starttime = sdl::timer::ticks();

    unsafe {
        naevc::threadpool_init();
        naevc::debug_sigInit();
    }

    if cfg!(unix) {
        /* Set window class and name. */
        unsafe {
            std::env::set_var("SDL_VIDEO_X11_WMCLASS", naev_core::APPNAME);
        }
    }

    let sdlvid = sdlctx.video()?;

    unsafe {
        naevc::nxml_init(); /* We'll be parsing XML. */
        naevc::input_init(); /* input has to be initialized for config to work. */
        naevc::conf_setDefaults(); /* set the default config values. */

        /*
         * Attempts to load the data path from datapath.lua
         * At this early point in the load process, the binary path
         * is the only place likely to be checked.
         */
        naevc::conf_loadConfigPath();
    }

    /* Create the home directory if needed. */
    let cpath = unsafe { naevc::nfile_configPath() };
    unsafe {
        if naevc::nfile_dirMakeExist(cpath) != 0 {
            warnx!(gettext("Unable to create config directory '{}'"), "foo");
        }
    }

    /* Set up the configuration. */
    let conf_file_path = unsafe {
        let rpath = cptr_to_cstr(cpath);
        let conf_file =
            CStr::from_ptr(naevc::CONF_FILE.as_ptr() as *const c_char).to_string_lossy();
        format!("{rpath}{conf_file}")
    };

    unsafe {
        let cconf_file_path = CString::new(conf_file_path.clone()).unwrap();
        naevc::conf_loadConfig(cconf_file_path.as_ptr()); /* Lua to parse the configuration file */
        naevc::conf_parseCLI(argv.len() as c_int, argv.as_mut_ptr()); /* parse CLI arguments */
    }

    // Will propagate error out if necessary
    ndata::setup()?;

    unsafe {
        /* Set up I/O. */
        naevc::gettext_setLanguage(naevc::conf.language); /* now that we can find translations */
        infox!(gettext("Loaded configuration: {}"), conf_file_path);
        let search_path = naevc::PHYSFS_getSearchPath();
        let mut buf = String::from(gettext("Read locations, searched in order:"));
        {
            let mut i = 0;
            loop {
                let sp = *search_path.offset(i);
                if sp.is_null() {
                    break;
                }
                buf.push_str(&format!("\n    {}", cptr_to_cstr(sp)));
                i += 1;
            }
        }
        info!("{buf}");
        naevc::PHYSFS_freeList(search_path as *mut c_void);

        /* Logging the cache path is noisy, noisy is good at the DEBUG level. */
        debugx!(
            gettext("Cache location: {}"),
            cptr_to_cstr(naevc::nfile_cachePath())
        );
        infox!(
            gettext("Write location: {}\n"),
            cptr_to_cstr(naevc::PHYSFS_getWriteDir())
        );
    }

    nlua::init()?;
    //let _lua = nlua::NLua::new()?;

    unsafe {
        /* Enable FPU exceptions. */
        if naevc::conf.fpu_except != 0 {
            naevc::debug_enableFPUExcept();
        }

        info!(
            " {}\n",
            CStr::from_ptr(naevc::start_name()).to_string_lossy()
        );

        /* Display the SDL version. */
        naevc::print_SDLversion();
    }

    /* Set up OpenGL. */
    let context = renderer::Context::new(sdlvid)?;

    unsafe {
        if naevc::gl_init() != 0 {
            let err = gettext("Initializing video output failed, exiting…");
            warn!("{err}");
            anyhow::bail!(err);
        }

        //Have to set up fonts before rendering anything.
        let font_prefix = naevc::FONT_PATH_PREFIX as *const u8 as *const i8;
        let font_default_path = gettext(
            "Cabin-SemiBold.otf,NanumBarunGothicBold.ttf,SourceCodePro-Semibold.ttf,IBMPlexSansJP-Medium.otf",
        );
        let font_default_path_c = CString::new(font_default_path).unwrap();
        let font_small_path = gettext(
            "Cabin-SemiBold.otf,NanumBarunGothicBold.ttf,SourceCodePro-Semibold.ttf,IBMPlexSansJP-Medium.otf",
        );
        let font_small_path_c = CString::new(font_small_path).unwrap();
        let font_mono_path =
            gettext("SourceCodePro-Semibold.ttf,D2CodingBold.ttf,IBMPlexSansJP-Medium.otf");
        let font_mono_path_c = CString::new(font_mono_path).unwrap();
        naevc::gl_fontInit(
            &raw mut naevc::gl_defFont,
            font_default_path_c.as_ptr(),
            naevc::conf.font_size_def as c_uint,
            font_prefix,
            0,
        );
        naevc::gl_fontInit(
            &raw mut naevc::gl_smallFont,
            font_small_path_c.as_ptr(),
            naevc::conf.font_size_small as c_uint,
            font_prefix,
            0,
        );
        naevc::gl_fontInit(
            &raw mut naevc::gl_defFontMono,
            font_mono_path_c.as_ptr(),
            naevc::conf.font_size_def as c_uint,
            font_prefix,
            0,
        );

        // Detect size changes that occurred after window creation.
        naevc::naev_resize();
    }

    // Display the initial load screen.
    let load_env = unsafe {
        let env = naevc::loadscreen_load();
        let s = CString::new(gettext("Initializing subsystems…")).unwrap();
        naevc::loadscreen_update(0., s.as_ptr());
        &*(env as *const nlua::LuaEnv)
    };

    // OpenAL
    unsafe {
        if naevc::conf.nosound != 0 {
            info!("{}", gettext("Sound is disabled!"));
            naevc::sound_disabled = 1;
            naevc::music_disabled = 1;
        }
        if naevc::sound_init() != 0 {
            warn!("{}", gettext("Problem setting up sound!"));
        }
        let m = CString::new("load")?;
        naevc::music_choose(m.as_ptr());
    }

    // Misc Init
    unsafe {
        naevc::fps_setPos(
            15.,
            (naevc::gl_screen.h - 15 - naevc::gl_defFontMono.h) as f64,
        );
    }

    /*
    {
        let path = "gfx/ship3d/Admonisher/admonisher.gltf";
        let ctx = crate::context::Context::get().as_safe_wrap();
        crate::model::Model::from_path(&ctx, &path)?;
    }
    */

    // Load game data
    load_all(&sdlctx, load_env)?;

    unsafe {
        // Detect size changes that occurred during load.
        naevc::naev_resize();

        // Unload load screen.
        naevc::loadscreen_unload();

        // Joystick
        if naevc::conf.joystick_ind >= 0 || !naevc::conf.joystick_nam.is_null() {
            if naevc::joystick_init() != 0 {
                warn!("{}", gettext("Error initializing joystick input"));
            }
            if !naevc::conf.joystick_nam.is_null() {
                if naevc::joystick_use(naevc::joystick_get(naevc::conf.joystick_nam)) != 0 {
                    warn!(
                        "{}",
                        gettext("Failure to open any joystick, falling back to default keybinds")
                    );
                    naevc::input_setDefault(1);
                }
            } else if naevc::conf.joystick_ind >= 0
                && naevc::joystick_use(naevc::conf.joystick_ind) != 0
            {
                warn!(
                    "{}",
                    gettext("Failure to open any joystick, falling back to default keybinds")
                );
                naevc::input_setDefault(1);
            }
        }

        // Start menu
        naevc::menu_main();

        if naevc::conf.devmode != 0 {
            infox!(
                gettext("Reached main menu in {:.3f} s"),
                (sdl::timer::ticks() - starttime) as f32 / 1000.
            );
        } else {
            info!("{}", gettext("Reached main menu"));
        }
        //NTracingMessageL( _( "Reached main menu" ) );

        // Initializes last_t
        naevc::fps_init();

        // Poll events
        // Flush events otherwise joystick loading can do weird things.
        let mut event_pump = sdlctx.event_pump().unwrap();
        for _ in event_pump.poll_iter() {}

        // Show plugin compatibility.
        naevc::plugin_check();
    }

    unsafe {
        naevc::naev_main_setup();
    }

    // Main loop
    while unsafe { naevc::naev_isQuit() } == 0 {
        unsafe {
            naevc::naev_main_events();
            naevc::main_loop(0);
        }

        // Process clean up messages
        context.execute_messages();
    }

    unsafe {
        naevc::naev_main_cleanup();
    }

    log::close_file();

    Ok(())
}

/// Small wrapper to handle loading
struct LoadStage {
    f: Box<dyn Fn() -> Result<()>>,
    msg: &'static str,
}
impl LoadStage {
    /// Loads data from a Rust function
    fn new<F>(msg: &'static str, f: F) -> LoadStage
    where
        F: Fn() -> Result<()> + 'static,
    {
        LoadStage {
            f: Box::new(f),
            msg,
        }
    }

    /// Loads data from a C function that gets wrapped
    fn new_c<F>(msg: &'static str, f: F) -> LoadStage
    where
        F: Fn() -> c_int + 'static,
    {
        LoadStage::new(msg, move || match f() {
            0 => Ok(()),
            _ => anyhow::bail!("Loading error!"),
        })
    }
}

fn load_all(sdlctx: &sdl::Sdl, env: &nlua::LuaEnv) -> Result<()> {
    unsafe {
        // Misc init stuff
        naevc::render_init();
        naevc::nebu_init();
        naevc::gui_init();
        naevc::toolkit_init();
        naevc::map_init();
        naevc::map_system_init();
        naevc::cond_init();
        naevc::cli_init();
        naevc::constants_init();
    }

    let stages: Vec<LoadStage> = vec![
        LoadStage::new_c(gettext("Loading Special Effects…"), || unsafe {
            naevc::spfx_load()
        }), /* no dep */
        LoadStage::new_c(gettext("Loading Effects…"), || unsafe {
            naevc::effect_load()
        }), /* no dep */
        LoadStage::new_c(gettext("Loading Factions…"), || unsafe {
            //faction::load().unwrap_or_else( |err| log::warn_err(err) );
            naevc::factions_load()
        }), /* dep for space, missions, AI, commodities */
        LoadStage::new_c(gettext("Loading Commodities…"), || unsafe {
            naevc::commodity_load()
        }), /* no dep */
        LoadStage::new_c(gettext("Loading Outfits…"), || unsafe {
            naevc::outfit_load()
        }), /* dep for ships, factions */
        LoadStage::new_c(gettext("Loading Ships…"), || unsafe {
            naevc::ships_load() + naevc::outfit_loadPost()
        }),
        LoadStage::new_c(gettext("Loading AI…"), || unsafe { naevc::ai_load() }), /* dep for ships, factions */
        LoadStage::new_c(gettext("Loading Techs…"), || unsafe {
            naevc::tech_load()
        }), /* dep for spobs */
        LoadStage::new_c(gettext("Loading the Universe…"), || unsafe {
            naevc::space_load()
        }), /* dep for events / missions */
        LoadStage::new_c(gettext("Loading Events and Missions…"), || unsafe {
            naevc::events_load() + naevc::missions_load()
        }),
        LoadStage::new_c(gettext("Loading UniDiffs…"), || unsafe {
            naevc::diff_init()
        }),
        LoadStage::new_c(gettext("Populating Maps…"), || unsafe {
            naevc::outfit_mapParse()
        }),
        LoadStage::new_c(gettext("Calculating Patrols…"), || unsafe {
            naevc::safelanes_init()
        }),
        // Run Lua and shit
        LoadStage::new_c(gettext("Finalizing data…"), || unsafe {
            //faction::load_lua().unwrap_or_else( |err| log::warn_err(err) );
            naevc::factions_loadPost()
                + naevc::difficulty_load()
                + naevc::background_init()
                + naevc::map_load()
                + naevc::map_system_load()
                + naevc::space_loadLua()
                + naevc::pilots_init()
                + naevc::weapon_init()
                + naevc::player_init()
        }),
    ];

    // Load one by one in order
    let mut stage: f32 = 0.0;
    let nstages: f32 = stages.len() as f32;
    let mut event_pump = sdlctx.event_pump().unwrap();
    for s in stages {
        loadscreen_update(env, (stage + 1.0) / (nstages + 2.0), s.msg).unwrap_or_else(|err| {
            log::warn_err(err.context("loadscreen failed to update!"));
        });
        stage += 1.0;
        (s.f)().unwrap_or_else(|err| {
            log::warn_err(err.context("loadscreen update function failed to run!"));
        });

        // Stops the window from going unresponsive
        for _ in event_pump.poll_iter() {}
    }

    loadscreen_update(env, 1.0, gettext("Loading Completed!")).unwrap_or_else(|err| {
        log::warn_err(err.context("loadscreen failed to update!"));
    });
    Ok(())
}

fn loadscreen_update(env: &nlua::LuaEnv, done: f32, msg: &str) -> Result<()> {
    let lua = &nlua::NLUA;
    let update: mlua::Function = env.get("update")?;
    env.call::<()>(lua, &update, (done, msg))?;
    unsafe {
        naevc::naev_doRenderLoadscreen();
    }
    Ok(())
}
