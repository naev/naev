use anyhow::Result;
use sdl2 as sdl;

use crate::log;

fn create_context(
    sdlvid: &sdl::VideoSubsystem,
    gl_attr: &sdl::video::gl_attr::GLAttr,
    major: u8,
    minor: u8,
) -> Result<(sdl::video::Window, sdl::video::GLContext)> {
    let (width, height, resizable, borderless) = unsafe {
        (
            naevc::conf.width,
            naevc::conf.height,
            naevc::conf.notresizable == 0,
            naevc::conf.borderless != 0,
        )
    };
    gl_attr.set_context_version(major, minor);

    let mut wdwbuild = sdlvid.window(
        crate::APPNAME,
        width.max(naevc::RESOLUTION_W_MIN),
        height.max(naevc::RESOLUTION_H_MIN),
    );
    if resizable {
        wdwbuild.resizable();
    }
    if borderless {
        wdwbuild.borderless();
    }
    wdwbuild.opengl().position_centered().allow_highdpi();
    let mut window = wdwbuild.build()?;

    window
        .set_minimum_size(naevc::RESOLUTION_W_MIN, naevc::RESOLUTION_H_MIN)
        .unwrap_or_else(|_| log::warn("Unable to set minimum window size."));
    let gl_context = match window.gl_create_context() {
        Ok(ctx) => ctx,
        Err(e) => anyhow::bail!("Unable to create OpenGL context: {}", e),
    };

    Ok((window, gl_context))
}

pub fn init(
    sdlvid: &sdl::VideoSubsystem,
) -> Result<(sdl::video::Window, sdl::video::GLContext, glow::Context)> {
    let (minimize, fsaa, vsync) = unsafe {
        (
            naevc::conf.minimize != 0,
            naevc::conf.fsaa,
            naevc::conf.vsync != 0,
        )
    };

    /* Focus behaviour. */
    sdl::hint::set_video_minimize_on_focus_loss(minimize);

    /* Set up the attributes. */
    let gl_attr = sdlvid.gl_attr();
    gl_attr.set_context_profile(sdl::video::GLProfile::Core);
    gl_attr.set_context_version(4, 7);
    gl_attr.set_double_buffer(true);
    if fsaa > 1 {
        gl_attr.set_multisample_buffers(1);
        gl_attr.set_multisample_samples(fsaa);
    }
    gl_attr.set_framebuffer_srgb_compatible(true);
    // TODO reenable debug mode
    // gl_attr.set_context_flags().debug().set();

    let (window, gl_context) = match create_context(sdlvid, &gl_attr, 4, 6) {
        Ok(v) => v,
        _ => match create_context(sdlvid, &gl_attr, 3, 2) {
            Ok(v) => v,
            _ => anyhow::bail!("Foo"),
        },
    };
    let gl = unsafe {
        glow::Context::from_loader_function(|s| sdlvid.gl_get_proc_address(s) as *const _)
    };

    // Final touches
    sdlvid
        .gl_set_swap_interval(match vsync {
            true => 1,
            false => 0,
        })
        .unwrap_or_else(|_| log::warn("Unable to set OpenGL swap interval!"));
    unsafe {
        naevc::gl_screen.window = window.raw() as *mut naevc::SDL_Window;
        naevc::gl_screen.context = gl_context.raw();
    }

    Ok((window, gl_context, gl))
}
