set $_exitcode = -999
set $print_once = 0
set height 0
handle SIGTERM nostop print pass
handle SIGPIPE nostop
define hook-stop
   if $_exitcode != -999
      quit
   else
      if $print_once == 0
         echo It seems like naev has crashed. Please debug with gdb now ;)\n
         set $print_once = 1
      end
   end
end
echo .gdbinit: running naev with gdb wrapper\n
run

