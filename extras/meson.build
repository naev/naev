if host_machine.system() == 'windows'
   meson.add_install_script(join_paths('windows','bundle.sh'))
endif

if host_machine.system() == 'darwin'
   meson.add_install_script(join_paths('macos','bundle.py'), debug ? '-d' : [])
endif

if get_option('release')
   if host_machine.system() == 'darwin'
      meson.add_install_script(join_paths('macos','packageMACOS.sh'))
   elif host_machine.system() == 'windows'
      meson.add_install_script(join_paths('windows','packageWindows.sh'))
   endif
endif
if get_option('installer')
   if host_machine.system() == 'darwin'
      meson.add_install_script(join_paths('macos','createDMG.sh'))
   elif host_machine.system() == 'windows'
      meson.add_install_script(join_paths('windows','createNSIS.sh'))
   endif
endif
