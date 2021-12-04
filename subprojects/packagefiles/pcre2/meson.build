project('pcre2', 'c', version: '10.23', license: 'BSD-3')

c_compiler = meson.get_compiler('c')

pcre2_h = configure_file(input : 'src/pcre2.h.generic',
  output : 'pcre2.h',
  configuration : configuration_data())

chartables = configure_file(input : 'src/pcre2_chartables.c.dist',
  output : 'pcre2_chartables.c',
  configuration : configuration_data())


config_h = configure_file(input : 'src/config.h.generic',
  output : 'config.h',
  configuration : configuration_data())

sources = [
  'src/pcre2_auto_possess.c',
  chartables,
  config_h, pcre2_h,
  'src/pcre2_compile.c',
  'src/pcre2_config.c',
  'src/pcre2_context.c',
  'src/pcre2_convert.c',
  'src/pcre2_dfa_match.c',
  'src/pcre2_error.c',
  'src/pcre2_extuni.c',
  'src/pcre2_find_bracket.c',
  'src/pcre2_jit_compile.c',
  'src/pcre2_jit_compile.c',
  'src/pcre2_maketables.c',
  'src/pcre2_match.c',
  'src/pcre2_match_data.c',
  'src/pcre2_newline.c',
  'src/pcre2_ord2utf.c',
  'src/pcre2_pattern_info.c',
  'src/pcre2_script_run.c',
  'src/pcre2_serialize.c',
  'src/pcre2_string_utils.c',
  'src/pcre2_study.c',
  'src/pcre2_substitute.c',
  'src/pcre2_substring.c',
  'src/pcre2_tables.c',
  'src/pcre2_ucd.c',
  'src/pcre2_valid_utf.c',
  'src/pcre2_xclass.c'
]

includes = [include_directories('.'), include_directories('src')]

check_headers = [
  ['HAVE_BZLIB_H', 'bzlib.h'],
  ['HAVE_SYS_STAT_H', 'sys/stat.h'],
  ['HAVE_SYS_TYPES_H', 'sys/types.h'],
  ['HAVE_SYS_WAIT_H', 'sys/wait.h'],
  ['HAVE_DIRENT_H', 'dirent.h'],
  ['HAVE_DLFCN_H', 'dlfcn.h'],
  ['HAVE_LIMITS_H', 'limits.h'],
  ['HAVE_STDLIB_H', 'stdlib.h'],
  ['HAVE_STRING_H', 'string.h'],
  ['HAVE_WINDOWS_H', 'windows.h'],
  ['HAVE_UNISTD_H', 'unistd.h']
]

config_h_defs = []
foreach h : check_headers
  if c_compiler.has_header(h.get(1))
    config_h_defs += ['-D' + h.get(0)]
  endif
endforeach


check_funs = [
  ['HAVE_BCOPY',        'strings.h',   'bcopy'         ],
  ['HAVE_MEMFD_CREATE', 'sys/mman.h',  'memfd_create'  ],
  ['HAVE_MEMMOVE',      'string.h',    'memmove'       ],
  ['HAVE_REALPATH',     'stdlib.h',    'realpath'      ],
  ['HAVE_SECURE_GETENV','stdlib.h',    'secure_getenv' ],
  ['HAVE_STRERROR',     'string.h',    'strerror'      ],
]

foreach f : check_funs
  if c_compiler.has_header_symbol(f.get(1), f.get(2))
    config_h_defs += ['-D' + f.get(0)]
  endif
endforeach


config_h_defs += ['-DHAVE_MEMMOVE', '-DSTDC_HEADERS', '-DSUPPORT_PCRE2_8', '-DSUPPORT_UNICODE']

pcre2_8_lib = library('pcre2-8', sources,
  include_directories: includes,
  c_args: config_h_defs + ['-DHAVE_CONFIG_H', '-DPCRE2_CODE_UNIT_WIDTH=8'],
  install: true)

pcre2_posix_lib = library('pcre2-posix', ['src/pcre2posix.c', 'src/pcre2posix.h'],
  include_directories: includes,
  link_with: pcre2_8_lib,
  c_args: config_h_defs + ['-DHAVE_CONFIG_H', '-DPCRE2_CODE_UNIT_WIDTH=8'],
  install: true)

pcre2_16_lib = library('pcre2-16', sources,
  include_directories: includes,
  c_args: config_h_defs + ['-DHAVE_CONFIG_H', '-DPCRE2_CODE_UNIT_WIDTH=16'],
  install: true)

pcre2_32_lib = library('pcre2-32', sources,
  include_directories: includes,
  c_args: config_h_defs + ['-DHAVE_CONFIG_H', '-DPCRE2_CODE_UNIT_WIDTH=32'],
  install: true)

libpcre2_8 = declare_dependency(link_with: pcre2_8_lib,
  include_directories: includes)
libpcre2_posix = declare_dependency(link_with: pcre2_posix_lib,
  include_directories: includes)
libpcre2_16 = declare_dependency(link_with: pcre2_16_lib,
  include_directories: includes)
libpcre2_32 = declare_dependency(link_with: pcre2_32_lib,
  include_directories: includes)

pcre2grep = executable('pcre2grep', ['src/pcre2grep.c'],
  include_directories: includes,
  link_with: [pcre2_8_lib],
  c_args: config_h_defs + ['-DHAVE_CONFIG_H'],
  install: true)

install_headers([pcre2_h, 'src/pcre2posix.h'])

########### pkg-config #############

pkg = import('pkgconfig')

pkg.generate(libraries : pcre2_8_lib,
             name : 'libpcre2-8',
             description: 'PCRE2 - Perl compatible regular expressions C library (2nd API) with 8 bit character support',
             version: meson.project_version())

pkg.generate(libraries : pcre2_16_lib,
             name : 'libpcre2-16',
             description: 'PCRE2 - Perl compatible regular expressions C library (2nd API) with 16 bit character support',
             version: meson.project_version())

pkg.generate(libraries : pcre2_32_lib,
             name : 'libpcre2-32',
             description: 'PCRE2 - Perl compatible regular expressions C library (2nd API) with 32 bit character support',
             version: meson.project_version())


#### tests

if not meson.is_cross_build() # wine wrappers are not bat-friendly, this would need better testing
  pcre2test = executable('pcre2test', ['src/pcre2test.c'],
    include_directories: includes,
    link_with: [pcre2_8_lib, pcre2_posix_lib, pcre2_16_lib, pcre2_32_lib],
    c_args: config_h_defs + ['-DHAVE_CONFIG_H'])

  if host_machine.system() == 'windows'
    runtest = find_program('RunTest.bat')
  else
    runtest = find_program('RunTest')
  endif

  test('RunTest', runtest,
    env : ['srcdir=@0@'.format(meson.current_source_dir())],
    workdir: meson.current_build_dir())
endif
