project('lua', 'c', version : '5.1.5', license : 'mit')

cc = meson.get_compiler('c')

system_deps = [cc.find_library('dl', required : false),
  cc.find_library('m', required : false)
]

if host_machine.system() == 'linux'
  add_project_arguments('-DLUA_USE_LINUX', language : 'c')
  readline_dep = cc.find_library('readline')
elif host_machine.system() == 'darwin'
  add_project_arguments('-DLUA_USE_MACOSX', language : 'c')
  readline_dep = cc.find_library('readline')
elif host_machine.system() == 'windows'
  add_project_arguments('-DLUA_USE_WINDOWS', language : 'c')
  readline_dep = []
else
  readline_dep = []
  error('Unsupported platform')
endif

incdir = include_directories('src')
subdir('src')

lua_dep = declare_dependency(link_with : lualib,
  include_directories : incdir)
