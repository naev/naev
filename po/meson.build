i18n = import('i18n')

add_project_arguments('-DGETTEXT_PACKAGE="' + meson.project_name() + '"', language:'c')

potfiles = custom_target(
    'potfiles',
    output: 'POTFILES.in',
    command: [ files('../utils/update-po.sh'), meson.source_root(), '@OUTPUT@' ],
    install: false
)

i18n.gettext(meson.project_name(),
    data_dirs: meson.source_root() / 'po',
    args: [
        '--directory=' + meson.source_root(),
        '--from-code=UTF-8',

        '--keyword=_',
        '--keyword=N_',
        '--keyword=gettext.gettext_noop',

        '--package-name=' + meson.project_name(),
        '--package-version=' + meson.project_version(),
        '--msgid-bugs-address=' + issue_address,
        '--copyright-holder=' + copyright_holder
        ]
)