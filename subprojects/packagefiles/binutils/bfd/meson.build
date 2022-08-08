# Roughly corresponding to bfd/configure.ac + includes, plus re-exports by Makefile.am.
AC_DEFINE = configuration_data()
AC_DEFINE.merge_from(features)
AC_SUBST = configuration_data()

sizeof_off_t = cc.sizeof('off_t')

AC_DEFINE.set_quoted('PACKAGE', 'bfd')
AC_DEFINE.set_quoted('PACKAGE_VERSION', meson.project_version())
if sizeof_off_t > 0
  AC_DEFINE.set('SIZEOF_OFF_T', sizeof_off_t)
endif
AC_DEFINE.set('HAVE_FCNTL', cc.has_function('fcntl'))
AC_DEFINE.set('HAVE_FOPEN', cc.has_function('fopen'))
AC_DEFINE.set('HAVE_FILENO', cc.has_function('fileno'))
AC_DEFINE.set10('HAVE_FLS', cc.has_function('fls'))
AC_DEFINE.set('HAVE_GETGID', cc.has_function('getgid'))
AC_DEFINE.set('HAVE_GETPAGESIZE', cc.has_function('getpagesize'))
AC_DEFINE.set('HAVE_GETRLIMIT', cc.has_function('getrlimit'))
AC_DEFINE.set('HAVE_GETUID', cc.has_function('getuid'))

AC_DEFINE.set10('HAVE_MADVISE', cc.has_header_symbol('sys/mman.h', 'madvise'))
AC_DEFINE.set('HAVE_MMAP', cc.has_header_symbol('sys/mman.h', 'mmap') ? 1 : false)
AC_DEFINE.set10('HAVE_MPROTECT', cc.has_header_symbol('sys/mman.h', 'mprotect'))

AC_DEFINE.set('USE_BINARY_FOPEN', host_machine.system() == 'windows')

# This is almost, but not quite, cc.has_attribute('visibility:hidden').
if cc.compiles('''
const char hw[] __attribute__ ((__visibility__ ("hidden"))) = "Hello, World\n";
extern void print (const char *) __attribute__ ((__visibility__ ("hidden")));
int main() { print (hw); }''', args: cc.get_supported_arguments(['-Werror=attributes']))
   AC_DEFINE.set('HAVE_HIDDEN', 1)
endif

AC_SUBST.set_quoted('BFD_INT64_FMT', cc.get_define('PRId64', prefix: '#include <inttypes.h>').strip('"d'))

AC_SUBST.set('wordsize', 64)
AC_SUBST.set('bfd_default_target_size', 64)
AC_SUBST.set('supports_plugins', 0)
AC_DEFINE.set('HAVE_FTELLO', cc.has_function('ftello'))
AC_DEFINE.set('HAVE_FTELLO64', cc.has_function('ftello64'))
AC_DEFINE.set('HAVE_FSEEKO', cc.has_function('fseeko'))
AC_DEFINE.set('HAVE_FSEEKO64', cc.has_function('fseeko64'))
AC_DEFINE.set('HAVE_FOPEN64', cc.has_function('fopen64'))
AC_DEFINE.set10('HAVE_DECL_FTELLO', cc.has_function('ftello'))
AC_DEFINE.set10('HAVE_DECL_FTELLO64', cc.has_function('ftello64'))
AC_DEFINE.set10('HAVE_DECL_FSEEKO', cc.has_function('fseeko'))
AC_DEFINE.set10('HAVE_DECL_FSEEKO64', cc.has_function('fseeko64'))
AC_DEFINE.set10('HAVE_DECL_FOPEN64', cc.has_function('fopen64'))
if sizeof_off_t == 8
  AC_SUBST.set('bfd_file_ptr', 'int64_t')
  AC_SUBST.set('bfd_ufile_ptr', 'uint64_t')
else
  AC_SUBST.set('bfd_file_ptr', 'long')
  AC_SUBST.set('bfd_ufile_ptr', 'unsigned long')
endif
if host_machine.system() == 'linux'  # FIXME wrong, but hard to execute test code in cross-builds.
  AC_DEFINE.set('USE_MMAP', 1)
endif

# This one's actually from Makefile.am.
AC_DEFINE.set_quoted('DEBUGDIR',  get_option('prefix') / get_option('libdir') / 'debug')

config_h = configure_file(output: 'config.h', configuration: AC_DEFINE)
bfd_h = configure_file(output: 'bfd.h', input: 'bfd-in2.h', configuration: AC_SUBST)

BUILD_CFILES = []
BUILD_HFILES = []
sed = find_program('sed')

BUILD_HFILES += custom_target('targmatch_h', output: 'targmatch.h', input: ['config.bfd', 'targmatch.sed'], command: [sed, '-f', '@INPUT1@', '@INPUT0@'], capture: true)

BUILD_HFILES += custom_target('elf32_target_h', output: 'elf32-target.h', input: 'elfxx-target.h', command: [sed, '-e', 's/NN/32/g', '@INPUT@'], capture: true)
BUILD_HFILES += custom_target('elf64_target_h', output: 'elf64-target.h', input: 'elfxx-target.h', command: [sed, '-e', 's/NN/64/g', '@INPUT@'], capture: true)

#FIXME: The Autotools build generated these next 12 source files with a prepended "#line" directive, too.
BUILD_CFILES += custom_target('elf32_aarch64_c', output: 'elf32-aarch64.c', input: 'elfnn-aarch64.c', command: [sed, '-e', 's/NN/32/g', '@INPUT@'], capture: true)
BUILD_CFILES += custom_target('elf64_aarch64_c', output: 'elf64-aarch64.c', input: 'elfnn-aarch64.c', command: [sed, '-e', 's/NN/64/g', '@INPUT@'], capture: true)
BUILD_CFILES += custom_target('elf32_ia64_c', output: 'elf32-ia64.c', input: 'elfnn-ia64.c', command: [sed, '-e', 's/NN/32/g', '@INPUT@'], capture: true)
BUILD_CFILES += custom_target('elf64_ia64_c', output: 'elf64-ia64.c', input: 'elfnn-ia64.c', command: [sed, '-e', 's/NN/64/g', '@INPUT@'], capture: true)
BUILD_CFILES += custom_target('elf32_loongarch_c', output: 'elf32-loongarch.c', input: 'elfnn-loongarch.c', command: [sed, '-e', 's/NN/32/g', '@INPUT@'], capture: true)
BUILD_CFILES += custom_target('elf64_loongarch_c', output: 'elf64-loongarch.c', input: 'elfnn-loongarch.c', command: [sed, '-e', 's/NN/64/g', '@INPUT@'], capture: true)
BUILD_CFILES += custom_target('elf32_riscv_c', output: 'elf32-riscv.c', input: 'elfnn-riscv.c', command: [sed, '-e', 's/NN/32/g', '@INPUT@'], capture: true)
BUILD_CFILES += custom_target('elf64_riscv_c', output: 'elf64-riscv.c', input: 'elfnn-riscv.c', command: [sed, '-e', 's/NN/64/g', '@INPUT@'], capture: true)

WTF = []
if host_machine.system() == 'windows'
   # Apparently, if you use thin LTO with MinGW (due to the object format, maybe),
   # and separate C files going into the static lib have identically-named static functions,
   # the build can die as follows: "lto1: error: two or more sections for .gnu.lto_<function name>.lto_priv.<some ID>".
   # Also, Meson can't give "sed" a command line containing backslashes without transforming them to slashes, because we live in hell.
   # BTW, all of this isn't enough, because the above elfnn-* files have their own identically named functions.
   WTF += ['-e', 's/abs_finder[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/add_data_entry[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/cleanup_syms[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/find_section_by_vma[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/is_vma_in_section[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/my_symbol_for_address[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/pe_is_repro[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/pe_print_debugdata[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/pe_print_[eip]data[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/pe_print_reloc[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_attach_chain[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_cmp[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_compute_region_sizes[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_compute_rva[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_count_directory[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_count_entries[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_merge[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_merge_string_entries[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_parse_directory[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_parse_entries[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_parse_entry[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_print_name[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_print_resource_directory[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_print_resource_entries[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_print_section[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_process_section[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_resource_name[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_sort_entries[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_write_directory[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_write_entry[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_write_leaf[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/rsrc_write_string[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/slurp_symtab[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/sort_x64_pdata[^a-zA-Z0-9_]/XX_&/']
   WTF += ['-e', 's/u16_mbtouc[^a-zA-Z0-9_]/XX_&/']
endif

BUILD_CFILES += custom_target('peigen_c', output: 'peigen.c', input: 'peXXigen.c', command: [sed, WTF, '-e', 's/XX/pe/g', '@INPUT@'], capture: true)
BUILD_CFILES += custom_target('pepigen_c', output: 'pepigen.c', input: 'peXXigen.c', command: [sed, WTF, '-e', 's/XX/pep/g', '@INPUT@'], capture: true)
BUILD_CFILES += custom_target('pex64', output: 'pex64.c', input: 'peXXigen.c', command: [sed, WTF, '-e', 's/XX/pex64/g', '@INPUT@'], capture: true)
BUILD_CFILES += custom_target('pe_aarch64igen_c', output: 'pe-aarch64igen.c', input: 'peXXigen.c', command: [sed, WTF, '-e', 's/XX/peAArch64/g', '@INPUT@'], capture: true)

bfdver_data = configuration_data()
bfdver_data.set('bfd_version', 238000000)
bfdver_data.set_quoted('bfd_version_package', '(GNU Binutils) ')
bfdver_data.set_quoted('bfd_version_string', meson.project_version())
bfdver_data.set_quoted('report_bugs_to', '<https://github.com/naev/naev/issues>')  # Assume bad things are my fault, not library authors'.
BUILD_HFILES += configure_file(output: 'bfdver.h', input: 'version.h', configuration: bfdver_data)

libbfd_includes = include_directories([
   '..',
   '.',
   '../include',
])

# Corresponding to the identically named lists in bfd/Makefile.am:
BFD32_LIBS_CFILES = [
	'archive.c', 'archures.c', 'bfd.c', 'bfdio.c', 'bfdwin.c', 'cache.c', 'coff-bfd.c',
	'compress.c', 'corefile.c', 'elf-properties.c', 'format.c', 'hash.c',
	'init.c', 'libbfd.c', 'linker.c', 'merge.c', 'opncls.c', 'reloc.c',
	'section.c', 'simple.c', 'stab-syms.c', 'stabs.c', 'syms.c', 'targets.c',
	'binary.c', 'ihex.c', 'srec.c', 'tekhex.c', 'verilog.c',
]

BFD64_LIBS_CFILES = ['archive64.c']

ALL_MACHINES_CFILES = [
	'cpu-aarch64.c',
	'cpu-alpha.c',
	'cpu-amdgcn.c',
	'cpu-arc.c',
	'cpu-arm.c',
	'cpu-avr.c',
	'cpu-bfin.c',
	'cpu-bpf.c',
	'cpu-cr16.c',
	'cpu-cris.c',
	'cpu-crx.c',
	'cpu-csky.c',
	'cpu-d10v.c',
	'cpu-d30v.c',
	'cpu-dlx.c',
	'cpu-epiphany.c',
	'cpu-fr30.c',
	'cpu-frv.c',
	'cpu-ft32.c',
	'cpu-h8300.c',
	'cpu-hppa.c',
	'cpu-i386.c',
	'cpu-iamcu.c',
	'cpu-ia64.c',
	'cpu-ip2k.c',
	'cpu-iq2000.c',
	'cpu-lm32.c',
	'cpu-loongarch.c',
	'cpu-m10200.c',
	'cpu-m10300.c',
	'cpu-m32c.c',
	'cpu-m32r.c',
	'cpu-m68hc11.c',
	'cpu-m68hc12.c',
	'cpu-m9s12x.c',
	'cpu-s12z.c',
	'cpu-m9s12xg.c',
	'cpu-m68k.c',
	'cpu-mcore.c',
	'cpu-mep.c',
	'cpu-metag.c',
	'cpu-microblaze.c',
	'cpu-mips.c',
	'cpu-mmix.c',
	'cpu-moxie.c',
	'cpu-msp430.c',
	'cpu-mt.c',
	'cpu-nds32.c',
	'cpu-nfp.c',
	'cpu-ns32k.c',
	'cpu-nios2.c',
	'cpu-or1k.c',
	'cpu-pdp11.c',
	'cpu-pj.c',
	'cpu-powerpc.c',
	'cpu-pru.c',
	'cpu-rs6000.c',
	'cpu-riscv.c',
	'cpu-rl78.c',
	'cpu-rx.c',
	'cpu-s390.c',
	'cpu-score.c',
	'cpu-sh.c',
	'cpu-sparc.c',
	'cpu-spu.c',
	'cpu-tic30.c',
	'cpu-tic4x.c',
	'cpu-tic54x.c',
	'cpu-tic6x.c',
	'cpu-tilegx.c',
	'cpu-tilepro.c',
	'cpu-v850.c',
	'cpu-v850_rh850.c',
	'cpu-vax.c',
	'cpu-visium.c',
	'cpu-wasm32.c',
	'cpu-xgate.c',
	'cpu-xstormy16.c',
	'cpu-xtensa.c',
	'cpu-z80.c',
	'cpu-z8k.c',
]

BFD32_BACKENDS_CFILES = [
	'aout-cris.c',
	'aout-ns32k.c',
	'aout32.c',
	'cf-i386lynx.c',
	'coff-go32.c',
	'coff-i386.c',
	'coff-mips.c',
	'coff-rs6000.c',
	'coff-sh.c',
	'coff-stgo32.c',
	'coff-tic30.c',
	'coff-tic4x.c',
	'coff-tic54x.c',
	'coff-z80.c',
	'coff-z8k.c',
	'coffgen.c',
	'cofflink.c',
	'dwarf1.c',
	'dwarf2.c',
	'ecoff.c',
	'ecofflink.c',
	'elf-attrs.c',
	'elf-eh-frame.c',
	'elf-ifunc.c',
	'elf-m10200.c',
	'elf-m10300.c',
	'elf-nacl.c',
	'elf-strtab.c',
	'elf-vxworks.c',
	'elf.c',
	'elf32-am33lin.c',
	'elf32-arc.c',
	'elf32-arm.c',
	'elf32-avr.c',
	'elf32-bfin.c',
	'elf32-cr16.c',
	'elf32-cris.c',
	'elf32-crx.c',
	'elf32-csky.c',
	'elf32-d10v.c',
	'elf32-d30v.c',
	'elf32-dlx.c',
	'elf32-epiphany.c',
	'elf32-fr30.c',
	'elf32-frv.c',
	'elf32-ft32.c',
	'elf32-gen.c',
	'elf32-h8300.c',
	'elf32-hppa.c',
	'elf32-i386.c',
	'elfxx-x86.c',
	'elf32-ip2k.c',
	'elf32-iq2000.c',
	'elf32-lm32.c',
	'elf32-m32c.c',
	'elf32-m32r.c',
	'elf32-m68hc11.c',
	'elf32-m68hc12.c',
	'elf32-m68hc1x.c',
	'elf32-m68k.c',
	'elf32-s12z.c',
	'elf32-mcore.c',
	'elf32-mep.c',
	'elf32-metag.c',
	'elf32-microblaze.c',
	'elf32-moxie.c',
	'elf32-msp430.c',
	'elf32-mt.c',
	'elf32-nds32.c',
	'elf32-nios2.c',
	'elf32-or1k.c',
	'elf32-pj.c',
	'elf32-ppc.c',
	'elf32-pru.c',
	'elf32-rl78.c',
	'elf32-rx.c',
	'elf32-s390.c',
	'elf32-sh.c',
	'elf32-sparc.c',
	'elf32-spu.c',
	'elf32-tic6x.c',
	'elf32-tilegx.c',
	'elf32-tilepro.c',
	'elf32-v850.c',
	'elf32-vax.c',
	'elf32-visium.c',
	'elf32-wasm32.c',
	'elf32-xgate.c',
	'elf32-xstormy16.c',
	'elf32-xtensa.c',
	'elf32-z80.c',
	'elf32.c',
	'elflink.c',
	'elfxx-sparc.c',
	'elfxx-tilegx.c',
	'i386aout.c',
	'i386bsd.c',
	'i386lynx.c',
	'i386msdos.c',
	'mach-o.c',
	'mach-o-i386.c',
	'mach-o-arm.c',
	'ns32knetbsd.c',
	'pc532-mach.c',
	'pdp11.c',
	'pe-arm-wince.c',
	'pe-arm.c',
	'pe-i386.c',
	'pe-mcore.c',
	'pe-sh.c',
	'pef.c',
	'pei-arm-wince.c',
	'pei-arm.c',
	'pei-i386.c',
	'pei-mcore.c',
	'pei-sh.c',
	'plugin.c',
	'ppcboot.c',
	'reloc16.c',
	'som.c',
	'vax1knetbsd.c',
	'vaxnetbsd.c',
	'vms-lib.c',
	'vms-misc.c',
	'wasm-module.c',
	'xcofflink.c',
	'xsym.c',
	'xtensa-isa.c',
	'xtensa-modules.c',
]

BFD64_BACKENDS_CFILES = [
	'aix5ppc-core.c',
	'aout64.c',
	'coff-alpha.c',
	'coff-x86_64.c',
	'coff64-rs6000.c',
	'elf32-mips.c',
	'elf32-score.c',
	'elf32-score7.c',
	'elf64-alpha.c',
	'elf64-amdgcn.c',
	'elf64-gen.c',
	'elf64-hppa.c',
	'elf64-ia64-vms.c',
	'elf64-mips.c',
	'elf64-mmix.c',
	'elf64-nfp.c',
	'elf64-ppc.c',
	'elf64-s390.c',
	'elf64-sparc.c',
	'elf64-tilegx.c',
	'elf64-x86-64.c',
	'elfxx-x86.c',
	'elf64-bpf.c',
	'elf64.c',
	'elfn32-mips.c',
	'elfxx-aarch64.c',
	'elfxx-ia64.c',
	'elfxx-loongarch.c',
	'elfxx-mips.c',
	'elfxx-riscv.c',
	'mach-o-aarch64.c',
	'mach-o-x86-64.c',
	'mmo.c',
	'pe-x86_64.c',
	'pei-aarch64.c',
	'pei-ia64.c',
	'pei-x86_64.c',
	'vms-alpha.c',
]

OPTIONAL_BACKENDS_CFILES = [
	'aix386-core.c',
	'cisco-core.c',
	'hpux-core.c',
	'irix-core.c',
	'lynx-core.c',
	'netbsd-core.c',
	'osf-core.c',
	'rs6000-core.c',
	'sco5-core.c',
	'trad-core.c',
]

SOURCE_CFILES = (
	BFD32_LIBS_CFILES
	+ BFD64_LIBS_CFILES
	+ ALL_MACHINES_CFILES
	+ BFD32_BACKENDS_CFILES
	+ BFD64_BACKENDS_CFILES
        # + OPTIONAL_BACKENDS_CFILES
)

CFILES = SOURCE_CFILES + BUILD_CFILES

libbfd_dependencies = [dependency('zlib'), cc.find_library('m', required: true), declare_dependency(sources: BUILD_HFILES)]

libbfd = static_library(
   'libbfd',
   CFILES,
   dependencies: libbfd_dependencies,
   include_directories: libbfd_includes,
 )
