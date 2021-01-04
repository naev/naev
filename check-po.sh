#!/bin/bash

set -xeuo pipefail

NUM_MESSAGES=$(msgfmt -o /dev/null --statistics "${POFILE}" 2>&1 | cut -f1 -d' ')
if (( NUM_MESSAGES > $EXPECT )); then
   exit 0
fi

cat <<EOF
It looks like some maintenance has damaged the translations.
The Naev 0.8 build environment (supporting Autotools and Meson, with "update-po" steps defined for each) is uniquely dangerous.
A known hazard is "make -C po update-po" (Autotools) using po/naev_src.pot instead of po/naev.pot as its reference.
There is no issue in the development version of Naev.
If you need to merge translation updates for one language (e.g., "de"), this obtuse recipe is known to work.
   cd po
   msgcat --lang=de --use-first <(git show revision_1:po/de.po) <(git show revision_2:po/de.po) ... > de.po
   INTLTOOL_EXTRACT="/usr/bin/intltool-extract" XGETTEXT="/usr/bin/xgettext" srcdir=. /usr/bin/intltool-update --gettext-package naev --dist -o de.new.po de
   mv de.new.po de.po
EOF
exit 1
