#!/bin/sh
cd @source_root@
type "gdb" > /dev/null
if [ "$?" == 0 ]; then
   gdb -x @source_root@/.gdbinit @naev_bin@ "$@"
else
   @naev_bin@ "$@"
fi
