#!/bin/sh

set -e

c="$1/tmpunwind.c"
o="$1/tmpunwind.o"
shift
cat >"${c}" <<\EOF
extern void b(void);
int a(void) {
  b();
  return 0;
}
EOF
"$@" -c "${c}" -o "${o}"
if grep -qa -e eh_frame -e __unwind_info "${o}" ||
    grep -qU -e eh_frame -e __unwind_info "${o}"; then
    echo E
fi

# vim: sw=4
