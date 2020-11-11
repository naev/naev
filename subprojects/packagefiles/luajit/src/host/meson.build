minilua = executable('minilua', 'minilua.c', dependencies: libm, install: false, native: true)

lj_arch_run = cc.run(lj_arch_test_source)
if not lj_arch_run.compiled() or lj_arch_run.returncode() != 0
    error('error running luajit arch testing program: ' + lj_arch_run.stderr())
endif
lj_arch_list = lj_arch_run.stdout().split(':')

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
    native: true
)
