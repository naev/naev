#!/bin/sh
type "gdb" > /dev/null
NAEV="@naev_bin@ -d @source_root@/dat -d @source_root@/artwork -d @source_root@"
if [ "$?" == 0 ]; then
   gdb -x @source_root@/.gdbinit --args ${NAEV} "$@"
else
   ${NAEV} "$@"
fi
