project('libbacktrace', 'c', version: '1.0')

cc = meson.get_compiler('c')
# Specific to Naev's warning levels.
c_args = cc.get_supported_arguments(['-Wno-pedantic'])
add_project_arguments(c_args, language: 'c')

c_args = cc.get_supported_arguments(['-funwind-tables', '-D_GNU_SOURCE'])
add_project_arguments(c_args, language: 'c')

config = configuration_data()
supported = configuration_data()

supported.set10('BACKTRACE_SUPPORTED', true)
config.set('HAVE_GETIPINFO', 1)
if cc.compiles('int main() { int i; __sync_bool_compare_and_swap (&i, i, i); __sync_lock_test_and_set (&i, 1); __sync_lock_release (&i); }')
  config.set('HAVE_SYNC_FUNCTIONS', 1)
endif
if cc.compiles('int main() { int i; __atomic_load_n (&i, __ATOMIC_ACQUIRE); __atomic_store_n (&i, 1, __ATOMIC_RELEASE); }')
  config.set('HAVE_ATOMIC_FUNCTIONS', 1)
endif
supported.set10('BACKTRACE_SUPPORTS_THREADS', config.get('HAVE_ATOMIC_FUNCTIONS', 0) == 1)

sys_filetype = run_command('filetype.sh', cc.cmd_array(), check: true).stdout().strip()
if sys_filetype.startswith('elf')
  FORMAT_FILE = 'elf.c'
  config.set('BACKTRACE_ELF_SIZE', sys_filetype.strip('elf')) # XXX - abusing .strip() to avoid requiring Meson >=0.58
  supported.set10('BACKTRACE_SUPPORTS_DATA', true)
elif sys_filetype == 'macho'
  FORMAT_FILE = 'macho.c'
  supported.set10('BACKTRACE_SUPPORTS_DATA', true)
elif sys_filetype == 'pecoff'
  FORMAT_FILE = 'pecoff.c'
  supported.set10('BACKTRACE_SUPPORTS_DATA', false)
elif sys_filetype.startswith('xcoff')
  FORMAT_FILE = 'xcoff.c'
  config.set('BACKTRACE_XCOFF_SIZE', sys_filetype.strip('xcoff')) # XXX - abusing .strip() to avoid requiring Meson >=0.58
  supported.set10('BACKTRACE_SUPPORTS_DATA', false)
else
  FORMAT_FILE = 'unknown.c'
  supported.set10('BACKTRACE_SUPPORTS_DATA', false)
endif

if cc.has_header_symbol('sys/mman.h', 'mmap')
  VIEW_FILE = 'mmapio.c'
  if cc.has_header_symbol('sys/mman.h', 'MAP_ANONYMOUS') or cc.has_header_symbol('sys/mman.h', 'MAP_ANON')
    ALLOC_FILE = 'mmap.c'
  else
    ALLOC_FILE = 'alloc.c'
  endif
else
  VIEW_FILE = 'read.c'
  ALLOC_FILE = 'alloc.c'
endif

supported.set10('BACKTRACE_USES_MALLOC', ALLOC_FILE == 'alloc.c')

config.set('HAVE_LINK_H', cc.has_header('link.h') ? 1 : false)
config.set('HAVE_SYS_LINK_H', cc.has_header('sys/link.h') ? 1 : false)
config.set('HAVE_DL_ITERATE_PHDR', cc.has_function('dl_iterate_phdr') ? 1 : false)
config.set('HAVE_MACH_O_DYLD_H', cc.has_header('mach-o/dyld.h') ? 1 : false)
config.set('HAVE_LOADQUERY', cc.has_function('loadquery') ? 1 : false)
config.set('HAVE_FCNTL', cc.has_function('fcntl') ? 1 : false)
config.set('HAVE_DECL_STRNLEN', cc.has_header_symbol('string.h', 'strnlen') ? 1 : false)
config.set('HAVE_DECL_GETPAGESIZE', cc.has_header_symbol('unistd.h', 'getpagesize') ? 1 : false)
config.set('HAVE_LSTAT', cc.has_function('lstat') ? 1 : false)
config.set('HAVE_READLINK', cc.has_function('readlink') ? 1 : false)
config.set('HAVE_GETEXECNAME', cc.has_function('getexecname') ? 1 : false)
if (cc.has_header_symbol('sys/sysctl.h', 'CTL_KERN', prefix: '#include <sys/types.h>') and
    cc.has_header_symbol('sys/sysctl.h', 'KERN_PROC_PATHNAME', prefix: '#include <sys/types.h>'))
   config.set('HAVE_KERN_PROC', cc.has_header_symbol('sys/sysctl.h', 'KERN_PROC', prefix: '#include <sys/types.h>') ? 1 : false)
   config.set('HAVE_KERN_PROC_ARGS', cc.has_header_symbol('sys/sysctl.h', 'KERN_PROC_ARGS', prefix: '#include <sys/types.h>') ? 1 : false)
endif

# Rely on Naev to link with any libraries needed for clock_gettime().
config.set('HAVE_CLOCK_GETTIME', cc.has_header_symbol('time.h', 'clock_gettime') ? 1 : false)

config_h = configure_file(output: 'config.h', configuration: config)
supported_h = configure_file(output: 'backtrace-supported.h', input: 'backtrace-supported.h.in', configuration: supported)
libbacktrace = static_library(
   'backtrace',
   ['atomic.c', 'dwarf.c', 'fileline.c', 'internal.h', 'posix.c', 'print.c', 'sort.c', 'state.c',
    'backtrace.c', 'simple.c', FORMAT_FILE, ALLOC_FILE, VIEW_FILE,
   ],
   dependencies: [declare_dependency(sources: [config_h, supported_h]), dependency('threads'), dependency('zlib')],
 )

libbacktrace_dep = declare_dependency(link_with: libbacktrace, include_directories: '.', version: meson.project_version())
