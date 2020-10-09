useNLS = get_option('nls')
if useNLS.disabled() == false
    i18n = import('i18n')
    gettext = find_program('gettext', required: useNLS)

    add_project_arguments('-DGETTEXT_PACKAGE="' + meson.project_name() + '"', language:'c')
    config_data.set10('ENABLE_NLS', true)

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
            ]
    )
endif
summary('Translations', gettext.found(), section: 'Features', bool_yn: true)