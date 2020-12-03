set $_exitcode = -999
set height 0
handle SIGTERM nostop print pass
handle SIGPIPE nostop
define hook-stop
    if $_exitcode != -999
        quit
    else
        echo It seems like naev has crashed. Please debug with gdb now ;)\n
    end
end
echo .gdbinit: running app\n
run

