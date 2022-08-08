project('binutils', 'c', version: '2.39')

cc = meson.get_compiler('c')
c_args = cc.get_supported_arguments([
   '-D_GNU_SOURCE', '-D_LARGEFILE_SOURCE', '-DHAVE_all_vecs',
])
add_project_arguments(c_args, language: 'c')

features = configuration_data()
features.set('HAVE_ALLOCA_H', cc.has_header('alloca.h') ? 1 : false)
features.set('HAVE_FCNTL_H', cc.has_header('fcntl.h') ? 1 : false)
features.set('HAVE_INTTYPES_H', cc.has_header('inttypes.h') ? 1 : false)
features.set('HAVE_LIMITS_H', cc.has_header('limits.h') ? 1 : false)
features.set10('HAVE_MACHINE_HAL_SYSINFO_H', cc.has_header('machine/hal_sysinfo.h'))
features.set('HAVE_MALLOC_H', cc.has_header('malloc.h') ? 1 : false)
features.set('HAVE_PROCESS_H', cc.has_header('process.h') ? 1 : false)
features.set('HAVE_STDINT_H', cc.has_header('stdint.h') ? 1 : false)
features.set('HAVE_STDIO_EXT_H', cc.has_header('stdio_ext.h') ? 1 : false)
features.set('HAVE_STDLIB_H', cc.has_header('stdlib.h') ? 1 : false)
features.set('HAVE_STRING_H', cc.has_header('string.h') ? 1 : false)
features.set('HAVE_STRINGS_H', cc.has_header('strings.h') ? 1 : false)
features.set('HAVE_SYS_FILE_H', cc.has_header('sys/file.h') ? 1 : false)
features.set('HAVE_SYS_MMAN_H', cc.has_header('sys/mman.h') ? 1 : false)
features.set('HAVE_SYS_PARAM_H', cc.has_header('sys/param.h') ? 1 : false)
features.set('HAVE_SYS_PRCTL_H', cc.has_header('sys/prctl.h') ? 1 : false)
features.set('HAVE_SYS_PSTAT_H', cc.has_header('sys/pstat.h') ? 1 : false)
features.set('HAVE_SYS_RESOURCE_H', cc.has_header('sys/resource.h') ? 1 : false)
features.set('HAVE_SYS_STAT_H', cc.has_header('sys/stat.h') ? 1 : false)
features.set('HAVE_SYS_SYSCTL_H', cc.has_header('sys/sysctl.h') ? 1 : false)
features.set10('HAVE_SYS_SYSINFO_H', cc.has_header('sys/sysinfo.h'))
features.set('HAVE_SYS_SYSMP_H', cc.has_header('sys/sysmp.h') ? 1 : false)
features.set('HAVE_SYS_SYSTEMCFG_H', cc.has_header('sys/systemcfg.h') ? 1 : false)
features.set('HAVE_SYS_TABLE_H', cc.has_header('sys/table.h') ? 1 : false)
features.set('HAVE_SYS_TIME_H', cc.has_header('sys/time.h') ? 1 : false)
features.set('HAVE_SYS_TYPES_H', cc.has_header('sys/types.h') ? 1 : false)
features.set('HAVE_SYS_WAIT_H', cc.has_header('sys/wait.h') ? 1 : false)
features.set('HAVE_TIME_H', cc.has_header('time.h') ? 1 : false)
features.set('HAVE_UNISTD_H', cc.has_header('unistd.h') ? 1 : false)
features.set('HAVE_VFORK_H', cc.has_header('vfork.h') ? 1 : false)

features.set10('HAVE_DECL_ASPRINTF', cc.has_function('asprintf'))
features.set10('HAVE_DECL_BASENAME', cc.has_function('basename'))
features.set10('HAVE_DECL_CALLOC', cc.has_function('calloc'))
features.set10('HAVE_DECL_FFS', cc.has_header_symbol('string.h', 'ffs'))
features.set10('HAVE_DECL_GETENV', cc.has_function('getenv'))
features.set10('HAVE_DECL_GETOPT', cc.has_function('getopt'))
features.set10('HAVE_DECL_MALLOC', cc.has_function('malloc'))
features.set10('HAVE_DECL_REALLOC', cc.has_function('realloc'))
# Above: a few "no shit, Sherlock" cases where we test for uniformity. Below: the autotools build has a rule about testing macOS sbrk(); we copy it.
features.set10('HAVE_DECL_SBRK', cc.has_function('sbrk') and host_machine.system() != 'darwin')
features.set10('HAVE_DECL_SNPRINTF', cc.has_function('snprintf'))
features.set10('HAVE_DECL_STPCPY', cc.has_header_symbol('string.h', 'stpcpy'))
features.set10('HAVE_DECL_STRNLEN', cc.has_function('strnlen'))
features.set10('HAVE_DECL_STRTOL', cc.has_function('strtol'))
features.set10('HAVE_DECL_STRTOLL', cc.has_function('strtoll'))
features.set10('HAVE_DECL_STRTOUL', cc.has_function('strtoul'))
features.set10('HAVE_DECL_STRTOULL', cc.has_function('strtoull'))
features.set10('HAVE_DECL_STRVERSCMP', cc.has_function('strverscmp'))
features.set10('HAVE_DECL_VASPRINTF', cc.has_function('vasprintf'))
features.set10('HAVE_DECL_VASNPRINTF', cc.has_function('vasnprintf'))

# Yes, we have a compiler from the current century.
features.set('HAVE_LONG_LONG', 1)
features.set('HAVE_INTPTR_T', 1)
features.set('HAVE_UINTPTR_T', 1)
features.set('STDC_HEADERS', 1)
features.set('UNSIGNED_64BIT_TYPE', 'uint64_t')
# But never trust Windows...
if host_machine.system() == 'windows'  # implying 64-bit Windows, for Naev's purposes
   features.set('pid_t', '__int64')
elif not cc.has_header('unistd.h') or cc.sizeof('pid_t', prefix: '#include <unistd.h>') < 0
   features.set('pid_t', 'int')
endif
if not cc.has_header('sys/types.h') or cc.sizeof('ssize_t', prefix: '#include <sys/types.h>') < 0
   features.set('ssize_t', 'int')
endif

features.set('SIZEOF_INT', cc.sizeof('int'))
features.set('SIZEOF_LONG', cc.sizeof('long'))
features.set('SIZEOF_LONG_LONG', cc.sizeof('long long'))
features.set('SIZEOF_SIZE_T', cc.sizeof('size_t'))
features.set('SIZEOF_VOID_P', cc.sizeof('void*'))

subdir('libiberty')
subdir('bfd')

libiberty_dep = declare_dependency(link_with: libiberty, include_directories: 'include', sources: BUILD_HFILES, version: meson.project_version())
libbfd_dep = declare_dependency(link_with: [libbfd, libiberty], include_directories: 'include', sources: BUILD_HFILES, version: meson.project_version())
