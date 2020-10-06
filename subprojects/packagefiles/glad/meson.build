project('glad', version : '0.1.33', license : 'mit')

glad_sh_in = files('glad.sh.in')
glad_sh = configure_file(
    input: glad_sh_in,
    output: 'glad.sh',
    configuration: {
        'path': meson.current_source_dir()
    }
)

meson.override_find_program('glad', glad_sh)