# The minor version number is from: `git show -s --format=%ct`.
project(
   'luajit',
   'c',
   license: 'MIT',
   version: '2.1.1720049189',
   meson_version: '>= 0.61.0',
)

fs = import('fs')
pkg = import('pkgconfig')

cc = meson.get_compiler('c')

foreach _define : [
   'LJ_ABI_SOFTFP',
   'LJ_ARCH_BITS',
   'LJ_ARCH_HASFPU',
   'LJ_ARCH_VERSION',
   'LJ_DUALNUM',
   'LJ_FR2',
   'LJ_HASFFI',
   'LJ_HASJIT',
   'LJ_LE',
   'LJ_NO_UNWIND',
   'LJ_TARGET_ARM',
   'LJ_TARGET_ARM64',
   'LJ_TARGET_X64',
   'LJ_TARGET_X86',
]
   set_variable(
      _define,
      cc.get_define(
         _define,
         include_directories: include_directories('src'),
         prefix: '#include "lj_arch.h"',
      ),
   )
endforeach

if LJ_TARGET_X64 != ''
   TARGET_LJARCH = 'x64'
elif LJ_TARGET_X86 != ''
   TARGET_LJARCH = 'x86'
elif LJ_TARGET_ARM != ''
   TARGET_LJARCH = 'arm'
elif LJ_TARGET_ARM64 != ''
   TARGET_LJARCH = 'arm64'
endif

DASM_ARCH = TARGET_LJARCH
if TARGET_LJARCH == 'x64' and LJ_FR2 != '1'
   DASM_ARCH = 'x86'
endif

DASM_FLAGS = []
TARGET_ARCH = []
TARGET_XCFLAGS = []

prefix = get_option('prefix')
if prefix != '/usr/local'
   if prefix == '/'
      prefix = ''
   endif
   TARGET_XCFLAGS += '-DLUA_ROOT="' + prefix + '"'
endif

if LJ_LE == '1'
   DASM_FLAGS += ['-D', 'ENDIAN_LE']
else
   DASM_FLAGS += ['-D', 'ENDIAN_BE']
endif
if LJ_ARCH_BITS == '64'
   DASM_FLAGS += ['-D', 'P64']
endif
if LJ_HASJIT == '1'
   DASM_FLAGS += ['-D', 'JIT']
endif
if LJ_HASFFI == '1'
   DASM_FLAGS += ['-D', 'FFI']
endif
if LJ_DUALNUM == '1'
   DASM_FLAGS += ['-D', 'DUALNUM']
endif
if LJ_ARCH_HASFPU == '1'
   DASM_FLAGS += ['-D', 'FPU']
   TARGET_ARCH += '-DLJ_ARCH_HASFPU=1'
else
   TARGET_ARCH += '-DLJ_ARCH_HASFPU=0'
endif
if LJ_ABI_SOFTFP != '1'
   DASM_FLAGS += ['-D', 'HFABI']
   TARGET_ARCH += '-DLJ_ABI_SOFTFP=0'
else
   TARGET_ARCH += '-DLJ_ABI_SOFTFP=1'
endif
if LJ_NO_UNWIND == '1'
   DASM_FLAGS += ['-D', 'NO_UNWIND']
   TARGET_ARCH += '-DLUAJIT_NO_UNWIND'
endif
DASM_FLAGS += ['-D', f'VER=@LJ_ARCH_VERSION@']

if host_machine.system() == 'darwin'
   unwind_external = true
else
   unwind_external = (
      LJ_NO_UNWIND != '1'
      and run_command(
         [find_program('./unwind_check.sh'), meson.project_build_root()] + cc.cmd_array(),
         check: true,
      ).stdout().strip() == 'E'
   )
endif
if unwind_external
   TARGET_XCFLAGS += '-DLUAJIT_UNWIND_EXTERNAL'
endif

TARGET_ARCH += f'-DLUAJIT_TARGET=LUAJIT_ARCH_@TARGET_LJARCH@'

HOST_XCFLAGS = []

if host_machine.system() in ['android', 'linux']
   LJVM_MODE = 'elfasm'
elif host_machine.system() == 'darwin'
   LJVM_MODE = 'machasm'
elif host_machine.system() == 'windows'
   LJVM_MODE = 'peobj'
   DASM_FLAGS += ['-D', 'WIN']
   HOST_XCFLAGS += '-malign-double'
else
   error('unsupported host system: ' + host_machine.system())
endif

if get_option('valgrind') and get_option('b_sanitize') != 'none'
   error('valgrind and b_sanitize=' + get_option('b_sanitize') + ' are not compatible')
endif

use_valgrind = get_option('valgrind')
if use_valgrind
   TARGET_XCFLAGS += ['-DLUAJIT_USE_VALGRIND']
endif
use_sysmalloc = (
   use_valgrind
   or get_option('sysmalloc')
   or get_option('b_sanitize') != 'none'
)
if use_sysmalloc
   TARGET_XCFLAGS += '-DLUAJIT_USE_SYSMALLOC'
endif

if get_option('lua52compat')
   TARGET_XCFLAGS += '-DLUAJIT_ENABLE_LUA52COMPAT'
endif

TARGET_XCFLAGS += cc.get_supported_arguments(
   '-fno-stack-protector',
   '-fomit-frame-pointer',
)

if meson.is_cross_build()
   _luajit_os = {
      'darwin' : 'OSX',
      'android': 'LINUX',
      'linux'  : 'LINUX',
      'windows': 'WINDOWS',
   }.get(host_machine.system(), 'OTHER')
   HOST_XCFLAGS += '-DLUAJIT_OS=LUAJIT_OS_' + _luajit_os
endif

if (
   cc.sizeof('void *') == 4
   and meson.get_compiler('c', native: true).sizeof('void *') == 8
)
   HOST_XCFLAGS += '-m32'
endif

m_dep = cc.find_library('m', required: false)
dl_dep = cc.find_library('dl', required: host_machine.system() == 'linux')

subdir('dynasm')
subdir('src')

summary(
   {
      'backward incompatible Lua 5.2 features': get_option('lua52compat'),
      'dynamic assembly architecture'         : DASM_ARCH,
      'target architecture'                   : TARGET_LJARCH,
      'use -fno-stack-protector'              : '-fno-stack-protector' in TARGET_XCFLAGS,
      'use -fomit-frame-pointer'              : '-fomit-frame-pointer' in TARGET_XCFLAGS,
      'use system malloc'                     : use_sysmalloc,
      'use valgrind'                          : use_valgrind,
      'unwind external'                       : unwind_external,
      'VM mode'                               : LJVM_MODE,
   },
   bool_yn: true,
)
