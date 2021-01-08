minilua = executable('minilua', 'minilua.c', dependencies: libm, install: false, native: true)
dasm_arch = []
buildvm_defines = []
dasm_args = [minilua, dynasm]

if cc.get_define('LJ_TARGET_X64', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir) == '1'
    lj_arch = 'x64'
elif cc.get_define('LJ_TARGET_X86', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir) == '1'
    lj_arch = 'x86'
elif cc.get_define('LJ_TARGET_ARM64', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir) == '1'
    lj_arch = 'arm64'
    if cc.get_define('__AARCH64EB__', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir) != ''
        buildvm_defines += '-D__AARCH64EB__=1'
    endif
elif cc.get_define('LJ_TARGET_ARM', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir) == '1'
    lj_arch = 'arm'
elif cc.get_define('LJ_TARGET_PPC', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir) == '1'
    lj_arch = 'ppc'
    if cc.get_define('LJ_LE', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir) == '1'
        buildvm_defines += '-DLJ_ARCH_ENDIAN=LUAJIT_LE'
    else
        buildvm_defines += '-DLJ_ARCH_ENDIAN=LUAJIT_BE'
    endif
elif cc.get_define('LJ_TARGET_MIPS64', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir) == '1'
    lj_arch = 'mips64'
    if cc.get_define('MIPSEL', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir) != ''
        buildvm_defines += '-D__MIPSEL__=1'
    endif
elif cc.get_define('LJ_TARGET_MIPS', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir) == '1'
    lj_arch = 'mips'
    if cc.get_define('MIPSEL', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir) != ''
        buildvm_defines += '-D__MIPSEL__=1'
    endif
else
    error('Unsupported architecture')
endif

buildvm_defines += '-DLUAJIT_TARGET=LUAJIT_ARCH_'+lj_arch

if ['windows', 'cygwin'].contains(host_machine.system())
    lj_os = 'WINDOWS'
elif ['linux'].contains(host_machine.system())
    lj_os = 'LINUX'
elif ['darwin'].contains(host_machine.system())
    lj_os = 'OSX'
elif ['dragonfly', 'freebsd', 'netbsd', 'openbsd'].contains(host_machine.system())
    lj_os = 'BSD'
elif ['gnu', 'sunos'].contains(host_machine.system())
    lj_os = 'POSIX'
else
    error('Sorry, your OS may be supported, but this meson.build file is incomplete. Please check the LuaJIT Makefiles.')
endif

buildvm_defines += '-DLUAJIT_OS=LUAJIT_OS_'+lj_os

if cc.get_define('LJ_LE', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir) == '1'
    dasm_args += ['-D', 'ENDIAN_LE']
else
    dasm_args += ['-D', 'ENDIAN_BE']
endif
if cc.get_define('LJ_ARCH_BITS', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir) == '64'
    dasm_args += ['-D', 'P64']
endif
if cc.get_define('LJ_HASJIT', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir) == '1'
    dasm_args += ['-D', 'JIT']
endif
if cc.get_define('LJ_HASFFI', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir) == '1'
    dasm_args += ['-D', 'FFI']
endif
if cc.get_define('LJ_DUALNUM', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir) == '1'
    dasm_args += ['-D', 'DUALNUM']
endif
if cc.get_define('LJ_ARCH_HASFPU', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir) == '1'
    dasm_args += ['-D', 'FPU']
    buildvm_defines += '-DLJ_ARCH_HASFPU=1'
else
    buildvm_defines += '-DLJ_ARCH_HASFPU=0'
endif
if cc.get_define('LJ_ABI_SOFTFP', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir) == '1'
    buildvm_defines += '-DLJ_ABI_SOFTFP=1'
else
    dasm_args += ['-D', 'HFABI']
    buildvm_defines += '-DLJ_ABI_SOFTFP=0'
endif
if cc.get_define('LJ_NO_UNWIND', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir) == '1'
    dasm_args += ['-D', 'NO_UNWIND']
    buildvm_defines += '-DLUAJIT_NO_UNWIND'
endif

dasm_args += ['-D', 'VER=' + cc.get_define('LJ_ARCH_VERSION', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir)]

if cc.get_define('_WIN32', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir) != ''
    dasm_args += ['-D', 'WIN']
endif

if cc.get_define('LJ_TARGET_MIPSR6', args: ['-include', 'lj_arch.h'], include_directories: luajit_source_dir) != ''
    dasm_args += ['-D', 'MIPSR6']
endif

dasm_args += ['-o', '@OUTPUT@', '@INPUT@']

vm_dasc = files('../vm_' + lj_arch + '.dasc')

buildvm_arch_h = custom_target('buildvm_arch.h',
    input : vm_dasc,
    output : 'buildvm_arch.h',
    command : dasm_args,
)

buildvm_sources = ['buildvm.c', 'buildvm_asm.c', 'buildvm_peobj.c', 'buildvm_lib.c', 'buildvm_fold.c']
buildvm_sources += buildvm_arch_h

buildvm = executable('buildvm',
    buildvm_sources,
    include_directories: luajit_source_dir,
    c_args: buildvm_defines,
    dependencies: libm,
    install: false,
    native: true
)
