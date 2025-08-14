# Building translations
i18n = import('i18n')

potfiles = custom_target(
   'potfiles',
   output: 'POTFILES.in',
   command: [ files('../utils/update-po.sh'), meson.project_source_root(), meson.project_build_root() ],
   install: false
)

i18n.gettext('naev',
   data_dirs: meson.project_source_root() / 'po',
   args: [
      '--files-from=' + (meson.project_source_root() / 'po/POTFILES.in'),
      '--directory=' + meson.project_source_root(),
      '--from-code=UTF-8',
      '--add-comments',

      '--keyword=_',
      '--keyword=N_',
      '--keyword=n_:1,2',
      '--keyword=p_:1c,2',
      '--keyword=gettext.gettext_noop',

      '--package-name=' + meson.project_name(),
      '--package-version=' + meson.project_version(),
      '--msgid-bugs-address=' + issue_address,
      '--copyright-holder=' + copyright_holder
      ],
   install_dir: ndata_path / 'dat/gettext',
)
