# No need for sanitizing and no point in wasting time with LTO on native helpers.
native_overrides = ['b_sanitize=none', 'b_lto=false']

minilua_exe = executable(
  'minilua',
  'minilua.c',
  c_args: HOST_XCFLAGS + TARGET_ARCH,
  dependencies: m_dep,
  link_args: HOST_XCFLAGS + TARGET_ARCH,
  native: true,
  override_options: native_overrides,
)

buildvm_arch_h = custom_target(
  'buildvm_arch.h',
  command: [
    minilua_exe,
    dynasm_lua,
  ] + DASM_FLAGS + [
    '-o', '@OUTPUT@',
    '@INPUT@',
  ],
  input: f'../vm_@DASM_ARCH@.dasc',
  output: 'buildvm_arch.h',
)

buildvm_exe = executable(
  'buildvm',
  [
    buildvm_arch_h,
    luajit_h,
    'buildvm_asm.c',
    'buildvm.c',
    'buildvm_fold.c',
    'buildvm_lib.c',
    'buildvm_peobj.c',
  ],
  c_args: HOST_XCFLAGS + TARGET_ARCH,
  include_directories: '..',
  link_args: HOST_XCFLAGS,
  native: true,
  override_options: native_overrides,
)
