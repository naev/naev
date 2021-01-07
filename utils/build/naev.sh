#!/bin/sh
"@source_root@"/utils/package-po.sh -b "@build_root@" -o "@build_root@"
NAEV="@naev_bin@ -d @source_root@/dat  -d @build_root@/dat -d @source_root@"
type "gdb" > /dev/null
if [ "$?" == 0 ]; then
   gdb -x @source_root@/.gdbinit --args ${NAEV} "$@"
else
   ${NAEV} "$@"
fi
