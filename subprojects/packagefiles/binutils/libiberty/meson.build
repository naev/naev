# Roughly corresponding to libiberty/configure.ac
config = configuration_data()
config.merge_from(features)

config.set_quoted('PACKAGE', 'bfd')
config.set_quoted('PACKAGE_VERSION', meson.project_version())
config.set('HAVE_BCOPY', cc.has_function('bcopy'))
config.set('HAVE_BZERO', cc.has_function('bzero'))
config.set('HAVE_CANONICALIZE_FILE_NAME', cc.has_function('canonicalize_file_name'))
config.set('HAVE_DUP3', cc.has_function('dup3'))
config.set('HAVE_GETCWD', cc.has_function('getcwd'))
config.set('HAVE_GETRLIMIT', cc.has_function('getrlimit'))
config.set('HAVE_GETRUSAGE', cc.has_function('getrusage'))
config.set10('HAVE_GETSYSINFO', cc.has_function('getsysinfo'))
config.set('HAVE_MEMCPY', cc.has_function('memcpy'))
config.set('HAVE_PIPE2', cc.has_function('pipe2'))
config.set('HAVE_PSIGNAL', cc.has_function('psignal'))
config.set10('HAVE_PSTAT_GETDYNAMIC', cc.has_function('pstat_getdynamic'))
config.set10('HAVE_PSTAT_GETSTATIC', cc.has_function('pstat_getstatic'))
config.set('HAVE_REALPATH', cc.has_function('realpath'))
config.set('HAVE_SBRK', features.get('HAVE_DECL_SBRK') == 1 ? 1 : false) # Special case...
config.set('HAVE_SETRLIMIT', cc.has_function('setrlimit'))
config.set('HAVE_SPAWNVE', cc.has_function('spawnve'))
config.set('HAVE_SPAWNVPE', cc.has_function('spawnvpe'))
config.set('HAVE_STRERROR', cc.has_function('strerror'))
config.set('HAVE_STRSIGNAL', cc.has_function('strsignal'))
config.set10('HAVE_SYSCTL', cc.has_function('sysctl'))
config.set10('HAVE_SYSMP', cc.has_function('sysmp'))
config.set10('HAVE_TABLE', cc.has_function('table'))
config.set('HAVE_TIMES', cc.has_function('times'))
config.set('HAVE_WAIT4', cc.has_function('wait4'))
config.set('HAVE_WAITPID', cc.has_function('waitpid'))
config.set('HAVE___FSETLOCKING', cc.has_function('__fsetlocking'))

config.set('HAVE_SYS_ERRLIST', cc.get_define('sys_errlist', prefix: '#include <stdlib.h>') != '')
config.set('HAVE_SYS_NERR', cc.get_define('sys_nerr', prefix: '#include <stdlib.h>') != '')

libiberty_includes = include_directories([
   '..',
   '.',
   '../include',
])

if host_machine.system() == 'windows'
   pexecute_c = 'pex-win32.c'
else
   pexecute_c = 'pex-unix.c'
endif

# Corresponding to REQUIRED_OFILES in libiberty/Makefile.in:
libiberty_source = [
	'regex.c', 'cplus-dem.c', 'cp-demangle.c',
	'md5.c', 'sha1.c', 'alloca.c',
	'argv.c',
	'bsearch_r.c',
	'choose-temp.c', 'concat.c',
	'cp-demint.c', 'crc32.c', 'd-demangle.c',
	'dwarfnames.c', 'dyn-string.c',
	'fdmatch.c', 'fibheap.c',
	'filedescriptor.c',
	'filename_cmp.c', 'floatformat.c',
	'fnmatch.c', 'fopen_unlocked.c',
	'getopt.c', 'getopt1.c', 'getpwd.c',
	'getruntime.c', 'hashtab.c', 'hex.c',
	'lbasename.c', 'lrealpath.c',
	'make-relative-prefix.c', 'make-temp-file.c',
	'objalloc.c',
	'obstack.c',
	'partition.c', 'pexecute.c', 'physmem.c',
	'pex-common.c', 'pex-one.c',
	pexecute_c, 'vprintf-support.c',
	'rust-demangle.c',
	'safe-ctype.c',
	'simple-object.c', 'simple-object-coff.c',
	'simple-object-elf.c', 'simple-object-mach-o.c',
	'simple-object-xcoff.c',
	'sort.c', 'spaces.c',
	'splay-tree.c', 'stack-limit.c',
	'strerror.c', 'strsignal.c',
	'timeval-utils.c', 'unlink-if-ordinary.c',
	'xasprintf.c', 'xatexit.c', 'xexit.c',
	'xmalloc.c', 'xmemdup.c', 'xstrdup.c',
	'xstrerror.c', 'xstrndup.c',
	'xvasprintf.c',
]

if features.get('HAVE_SBRK', 0) != 1
   libiberty_source += 'ffs.c'
endif

'''
AC_C_BIGENDIAN
AC_HEADER_SYS_WAIT
AC_HEADER_TIME

libiberty_AC_DECLARE_ERRNO

funcs="asprintf atexit basename bcmp bcopy bsearch bzero calloc clock ffs getcwd getpagesize gettimeofday index insque memchr memcmp memcpy memmem memmove mempcpy memset mkstemps putenv random rename rindex setenv snprintf sigsetmask stpcpy stpncpy strcasecmp strchr strdup strncasecmp strndup strnlen strrchr strstr strtod strtol strtoul strtoll strtoull strverscmp tmpnam vasprintf vfprintf vprintf vsnprintf vsprintf waitpid setproctitle"

# Also in the old function.def file: alloca, vfork, getopt.
AC_CHECK_FUNCS("gettimeofday on_exit sysconf wait3")

# For each of these functions, if the host does not provide the function we want to put FN.o in LIBOBJS, and if the host does provide the function, we want to define HAVE_FN in config.h.

AC_ISC_POSIX
AC_REPLACE_FUNCS($funcs)
libiberty_AC_FUNC_C_ALLOCA
AC_FUNC_FORK
if test $ac_cv_func_vfork_works = no; then
  AC_LIBOBJ([vfork])
fi
# We only need _doprnt if we might use it to implement v*printf.
if test $ac_cv_func_vprintf != yes \
   || test $ac_cv_func_vfprintf != yes \
   || test $ac_cv_func_vsprintf != yes; then
  AC_REPLACE_FUNCS(_doprnt)
else
  AC_CHECK_FUNCS(_doprnt)
fi

libiberty_NEED_DECLARATION(canonicalize_file_name)

libiberty_AC_FUNC_STRNCMP
'''

config_h = configure_file(output: 'config.h', configuration: config)
libiberty_dependencies = [declare_dependency(sources: config_h)]

libiberty = static_library(
   'libiberty',
   libiberty_source,
   c_args: ['-include', 'stdlib.h', '-DHAVE_CONFIG_H'],
   dependencies: libiberty_dependencies,
   include_directories: libiberty_includes,
 )
