src = [
  'lapi.c',
  'lauxlib.c',
  'lbaselib.c',
  'lcode.c',
  'ldblib.c',
  'ldebug.c',
  'ldo.c',
  'ldump.c',
  'lfunc.c',
  'lgc.c',
  'linit.c',
  'liolib.c',
  'llex.c',
  'lmathlib.c',
  'lmem.c',
  'loadlib.c',
  'lobject.c',
  'lopcodes.c',
  'loslib.c',
  'lparser.c',
  'lstate.c',
  'lstring.c',
  'lstrlib.c',
  'ltable.c',
  'ltablib.c',
  'ltm.c',
  'lundump.c',
  'lvm.c',
  'lzio.c',
  'print.c'
]

lualib = library('lua', src,
  dependencies : system_deps)

executable('luai', 'lua.c',
  link_with : lualib,
  dependencies : readline_dep)

# This only works with static linking.
if get_option('default_library') == 'static'
  executable('luac', 'luac.c',
    link_with : lualib)
endif
