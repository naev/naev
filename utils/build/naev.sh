#!/usr/bin/env bash

# Prefers GDB over LLDB if both are installed, you can choose your preference
# by exporting PREFERLLDB=true

# Set WITHDEBUGGER=false to avoid using debuggers where it is a hinderance.

get_debugger() {
   local preferred_debugger="gdb"

   if [ "$WITHDEBUGGER" = "false" ]; then
      echo "Debugging disabled."
      return
   fi

   if [ "$PREFERLLDB" = "true" ]; then
      preferred_debugger="lldb"
   fi

   if [ -n "$preferred_debugger" ] && command -v "$preferred_debugger" &> /dev/null; then
      echo "$preferred_debugger"
   elif command -v lldb &> /dev/null; then
      echo "lldb"
   elif command -v gdb &> /dev/null; then
      echo "gdb"
   else
      echo "Error: Neither lldb nor gdb is installed. Debugging disabled."
      return
   fi
}

"@source_root@/meson.sh" compile -C "@build_root@" naev-gmo
mkdir -p "@build_root@"/dat/gettext
# Meson <= 0.59
for mo_path in "@build_root@"/po/*.gmo; do
   if [ -f "$mo_path" ]; then
      mo_name="$(basename "$mo_path")"
      lang=${mo_name%.gmo}
      mkdir -p "@build_root@/dat/gettext/$lang/LC_MESSAGES"
      cp -v "$mo_path" "@build_root@/dat/gettext/$lang/LC_MESSAGES/naev.mo"
   fi
done
# Meson >= 0.60
for mo_path in "@build_root@"/po/*/; do
   if [ -d "$mo_path" ]; then
      cp -vr "$mo_path" "@build_root@"/dat/gettext
   fi
done

# shellcheck disable=SC2050
wrapper() {
   if [[ "@debug_paranoid@" = "True" ]]; then
      export ALSOFT_LOGLEVEL=3
      export ALSOFT_TRAP_AL_ERROR=1
   elif [[ "@debug@" = "True" ]]; then
      export ALSOFT_LOGLEVEL=2
   fi
   export ASAN_OPTIONS=halt_on_error=1
   DEBUGGER=$(get_debugger)
   if [[ $DEBUGGER =~ "gdb" ]]; then
      exec $DEBUGGER -x "@build_root@/.gdbinit" --args "$@"
   elif [[ $DEBUGGER =~ "lldb" ]]; then
      # TODO: make something similar to the gdbinit setup with lldbinit
      exec $DEBUGGER -o run -- "$@"
   else
      exec "$@"
   fi
}

wrapper "@naev_bin@" -d "@zip_overlay@" -d "@source_root@/dat" -d "@source_root@/artwork" -d "@build_root@/dat" -d "@source_root@" "$@"
