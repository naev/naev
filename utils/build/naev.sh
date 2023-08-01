#!/bin/bash

# export WITHGDB=NO to avoid using GDB where it is a hinderance.

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
   if [[ ! "$WITHGDB" =~ "NO" ]] && type "gdb" > /dev/null 2>&1; then
      export ASAN_OPTIONS=abort_on_error=1
      exec gdb -x "@build_root@/.gdbinit" --args "$@"
   else
      exec "$@"
   fi
}

wrapper "@naev_bin@" -d "@zip_overlay@" -d "@source_root@/dat" -d "@source_root@/artwork" -d "@build_root@/dat" -d "@source_root@" "$@"
