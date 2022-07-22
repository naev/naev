set height 0
handle SIGTERM nostop print pass
handle SIGPIPE nostop
# Tell gdb to detach before a normal exit, so that LeakSanitizer can do its job.
break exit
commands
   detach
end
echo .gdbinit: running naev with gdb wrapper\n
run
