if host_machine.system() == 'windows'
   if get_option('installer')
      meson.add_install_script(join_paths('windows','packageWindows.sh'), debug ? '-n' : [])
   endif
endif

if host_machine.system() == 'darwin'
   meson.add_install_script(join_paths('macos','bundle.py'), debug ? '-d' : [])
   # Archive Naev.app to improve speed of release process. Release scripts depend on this being done.
   if get_option('release')
      meson.add_install_script(join_paths('macos','packageMACOS.sh'))
   endif
   if get_option('installer')
      meson.add_install_script(join_paths('macos','createDMG.sh'))
   endif
endif
