useNLS = get_option('nls')

hasNGetText = false
if useNLS.disabled() == false
    # Naev Deps
    if cc.has_function('ngettext')
        hasNGetText = true
    else
        libintl = cc.find_library('intl', required: useNLS)
        hasNGetText = libintl.found()
        naev_deps += libintl
    endif

    if hasNGetText
        add_project_arguments('-DGETTEXT_PACKAGE="' + meson.project_name() + '"', language:'c')
    endif

    # Building translations
    i18n = import('i18n')

    potfiles = custom_target(
        'potfiles',
        output: 'POTFILES.in',
        command: [ files('../utils/update-po.sh'), meson.source_root(), '@OUTPUT@' ],
        install: false
    )

    i18n.gettext(meson.project_name(),
        data_dirs: meson.source_root() / 'po',
        args: [
            '--files-from=' + (meson.source_root() / 'po/POTFILES_COMBINED.in'),
            '--directory=' + meson.source_root(),
            '--from-code=UTF-8',
            '--add-comments',

            '--keyword=_',
            '--keyword=N_',
            '--keyword=gettext.gettext_noop',

            '--package-name=' + meson.project_name(),
            '--package-version=' + meson.project_version(),
            '--msgid-bugs-address=' + issue_address,
            '--copyright-holder=' + copyright_holder
            ],
        install_dir: ndata_path / 'dat/gettext',
    )

    # LANGUAGES file generation
    generate_languages = run_command(gen_languages)
    if generate_languages.returncode() != 0
    error(generate_languages.stderr())
    else
    meson.add_dist_script('../'+add_to_package, 'dat/LANGUAGES')
    endif
endif

config_data.set10('ENABLE_NLS', hasNGetText)
summary('Translations', hasNGetText, section: 'Features', bool_yn: true)
