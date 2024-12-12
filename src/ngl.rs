use anyhow::Result;
use sdl2 as sdl;

pub fn init(sdlvid: &sdl::VideoSubsystem) -> Result<()> {
    let (width, height, minimize, fsaa, resizable, borderless) = unsafe {
        (
            naevc::conf.width,
            naevc::conf.height,
            naevc::conf.minimize != 0,
            naevc::conf.fsaa,
            naevc::conf.notresizable == 0,
            naevc::conf.borderless != 0,
        )
    };

    /* Focus behaviour. */
    sdl::hint::set_video_minimize_on_focus_loss(minimize);

    let gl_attr = sdlvid.gl_attr();
    gl_attr.set_context_profile(sdl::video::GLProfile::Core);
    gl_attr.set_context_version(3, 2);
    gl_attr.set_double_buffer(true);
    if fsaa > 1 {
        gl_attr.set_multisample_buffers(1);
        gl_attr.set_multisample_samples(fsaa);
    }
    gl_attr.set_framebuffer_srgb_compatible(true);
    // gl_attr.set_context_flags().debug().set();
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
        .unwrap();
    let gl_context = window.gl_create_context().unwrap();
    let gl = unsafe {
        glow::Context::from_loader_function(|s| sdlvid.gl_get_proc_address(s) as *const _)
    };

    Ok(())
}
