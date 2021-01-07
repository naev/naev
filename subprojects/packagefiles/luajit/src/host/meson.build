minilua = executable('minilua', 'minilua.c', dependencies: libm, install: false, native: true)

lj_arch_list = ['x64', 'x64', '-DLUAJIT_TARGET=LUAJIT_ARCH_x64,-DLUAJIT_OS=LUAJIT_OS_OSX,-DLJ_ARCH_HASFPU=1,-DLJ_ABI_SOFTFP=0', '-D,ENDIAN_LE,-D,P64,-D,JIT,-D,FFI,-D,FPU,-D,HFABI,-D,VER=']

# Print output of lj_arch_test for information purpose.
foreach line : lj_arch_list
    message(line)
endforeach

lj_arch = lj_arch_list[0]
dasm_arch = lj_arch_list[1]
buildvm_defines = lj_arch_list[2].split(',')

dasm_args = [minilua, dynasm]
dasm_args += lj_arch_list[3].split(',')
dasm_args += ['-o', '@OUTPUT@', '@INPUT@']

vm_dasc = files('../vm_' + dasm_arch + '.dasc')

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
    native: true,
    override_options: ['b_sanitize=none'], # :(
)
