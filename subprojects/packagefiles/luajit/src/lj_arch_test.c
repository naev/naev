#include <stdio.h>
#include "lj_arch.h"

/* Print in stdout the following string:

   <TARGET_LJARCH>:<DASM ARCH>:<TARGET_ARCH>:<DASM_AFLAGS>

   Where <TARGET_ARCH> are <DASM_AFLAGS> comma separated strings.

   Each value correspond to the variable of the same name in Luajit's
   original Makefile.
*/

/* NB This file is intentionally primitive and no dynamic memory is used.
   It is not supposed to be a good, general purpose, programming style
   but it is good enough to print some architecture details. */

#define MAIN_SEPARATOR ":"
#define DEFS_SEPARATOR ","

static void add_def(const char *args[], int *args_n, const char *def) {
	const int n = *args_n;
	args[n] = "-D";
	args[n + 1] = def;
	*args_n = n + 2;
}

int main() {
const char *lj_arch, *lj_os, *dasm_arch;
const char *arch_defs[16];
int arch_defs_n = 0;
const char *x_arch_option = NULL;

#ifdef LJ_TARGET_X64
lj_arch = "x64";
#elif LJ_TARGET_X86
lj_arch = "x86";
#elif LJ_TARGET_ARM
lj_arch = "arm";
#elif LJ_TARGET_PPC
lj_arch = "ppc";
#elif LJ_TARGET_PPCSPE
lj_arch = "ppcspe";
#elif LJ_TARGET_MIPS
lj_arch = "mips";
#ifdef MIPSEL
x_arch_option = "-D__MIPSEL__=1";
#endif
#else
fprintf(stderr, "Unsupported architecture\n");
exit(1);
#endif

char luajit_target_def[128];
sprintf(luajit_target_def, "-DLUAJIT_TARGET=LUAJIT_ARCH_%s", lj_arch);
arch_defs[arch_defs_n++] = luajit_target_def;

if (x_arch_option) {
    arch_defs[arch_defs_n++] = x_arch_option;
}

#if LUAJIT_OS == LUAJIT_OS_OTHER
lj_os = "OTHER";
#elif LUAJIT_OS == LUAJIT_OS_WINDOWS
lj_os = "WINDOWS";
#elif LUAJIT_OS == LUAJIT_OS_LINUX
lj_os = "LINUX";
#elif LUAJIT_OS == LUAJIT_OS_OSX
lj_os = "OSX";
#elif LUAJIT_OS == LUAJIT_OS_BSD
lj_os = "BSD";
#elif LUAJIT_OS == LUAJIT_OS_POSIX
lj_os = "POSIX";
#else
fprintf(stderr, "Unsupported OS\n");
exit(1);
#endif

char luajit_os_def[128];
sprintf(luajit_os_def, "-DLUAJIT_OS=LUAJIT_OS_%s", lj_os);
arch_defs[arch_defs_n++] = luajit_os_def;

#ifdef LJ_TARGET_X64
dasm_arch = "x86";
#else
dasm_arch = lj_arch;
#endif

const char *dasm[32];
int dasm_n = 0;

#if LJ_ARCH_BITS == 64
add_def(dasm, &dasm_n, "P64");
#endif
#if LJ_HASJIT == 1
add_def(dasm, &dasm_n, "JIT");
#endif
#if LJ_HASFFI == 1
add_def(dasm, &dasm_n, "FFI");
#endif
#if LJ_DUALNUM == 1
add_def(dasm, &dasm_n, "DUALNUM");
#endif
#if LJ_ARCH_HASFPU == 1
add_def(dasm, &dasm_n, "FPU");
arch_defs[arch_defs_n++] = "-DLJ_ARCH_HASFPU=1";
#else
arch_defs[arch_defs_n++] = "-DLJ_ARCH_HASFPU=0";
#endif
#if LJ_ABI_SOFTFP == 1
arch_defs[arch_defs_n++] = "-DLJ_ABI_SOFTFP=1";
#else
add_def(dasm, &dasm_n, "HFABI");
arch_defs[arch_defs_n++] = "-DLJ_ABI_SOFTFP=0";
#endif
#if LJ_NO_UNWIND == 1
add_def(dasm, &dasm_n, "NO_UNWIND");
arch_defs[arch_defs_n++] = "-DLUAJIT_NO_UNWIND";
#endif

char arm_arch_version[16];
#if LJ_ARCH_VERSION
sprintf(arm_arch_version, "VER=%d", LJ_ARCH_VERSION);
#else
sprintf(arm_arch_version, "VER=");
#endif
add_def(dasm, &dasm_n, arm_arch_version);

#ifdef _WIN32
add_def(dasm, &dasm_n, "WIN");
#endif

#if (!defined LJ_TARGET_X64 && defined LJ_TARGET_X86) && __SSE2__ == 1
add_def(dasm, &dasm_n, "SSE");
#endif

printf("%s%s%s%s", lj_arch, MAIN_SEPARATOR, dasm_arch, MAIN_SEPARATOR);
for (int i = 0; i < arch_defs_n; i++) {
	printf("%s%s", i > 0 ? DEFS_SEPARATOR : "", arch_defs[i]);
}
printf("%s", MAIN_SEPARATOR);
for (int i = 0; i < dasm_n; i++) {
	printf("%s%s", i > 0 ? DEFS_SEPARATOR : "", dasm[i]);
}

return 0;
}
