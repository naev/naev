#!/bin/bash
NAEV="@naev_bin@ -d @source_root@/dat -d @source_root@/artwork -d @build_root@/dat -d @source_root@"

"@source_root@/meson.sh" compile -C "@build_root@" naev-gmo
for mo_path in "@build_root@"/po/*.gmo; do
    mo_name="$(basename "$mo_path")"
    lang=${mo_name%.gmo}
    mkdir -p "@build_root@"/dat/gettext/$lang/LC_MESSAGES
    cp -v "$mo_path" "@build_root@"/dat/gettext/$lang/LC_MESSAGES/naev.mo
done
grep -q '%<PRI' "@build_root@"/po/*.gmo && echo "***WARNING: Naev can't translate 'sysdep' strings like %<PRIu64>. Try %.0f?"

type "gdb" > /dev/null
if [ "$?" == 0 ]; then
   gdb -x @source_root@/.gdbinit --args ${NAEV} "$@"
else
   ${NAEV} "$@"
fi
