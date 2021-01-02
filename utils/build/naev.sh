#!/bin/sh
type "gdb" > /dev/null
if [ "$?" == 0 ]; then
   gdb -x @source_root@/.gdbinit --args @naev_bin@ -d @source_root@/dat -d @source_root@ "$@"
else
   @naev_bin@ -d @source_root@/dat "$@"
fi
