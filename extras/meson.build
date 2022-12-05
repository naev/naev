if host_machine.system() == 'windows'
   if get_option('installer')
      meson.add_install_script(join_paths('windows','packageWindows.sh'), debug ? '-n' : [])
   endif
endif

if host_machine.system() == 'darwin'
   meson.add_install_script(join_paths('macos','bundle.py'), debug ? '-d' : [])
   if get_option('installer')
      meson.add_install_script(join_paths('macos','createDMG.sh'))
   endif
endif
