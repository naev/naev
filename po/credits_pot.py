#!/usr/bin/env python3
import json
import os
import re
import sys

HEADER = r"""# Naev Credits
# Copyright (C) YEAR Naev Dev Team
# This file is distributed under the same license as the naev package.
# FIRST AUTHOR <EMAIL@ADDRESS>, YEAR.
#
#, fuzzy
msgid ""
msgstr ""
"Project-Id-Version: PACKAGE VERSION\n"
"Report-Msgid-Bugs-To: https://github.com/naev/naev/issues\n"
"POT-Creation-Date: YEAR-MO-DA HO:MI+ZONE\n"
"PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\n"
"Last-Translator: FULL NAME <EMAIL@ADDRESS>\n"
"Language-Team: LANGUAGE <LL@li.org>\n"
"Language: \n"
"MIME-Version: 1.0\n"
"Content-Type: text/plain; charset=UTF-8\n"
"Content-Transfer-Encoding: 8bit\n"
"""

with open(sys.argv[1],"w") as fout:
    for fn in sys.argv[2:]:
        fout.write(HEADER)
        with open(fn) as f:
            fn = re.sub('.*/dat', 'dat', fn)
            for i, line in enumerate(f):
                line = line.rstrip('\r\n')
                if line and not line.startswith('['):
                    quoted_escaped_line = json.dumps(line, ensure_ascii=False)
                    fout.write('#: {}:{}\nmsgid {}\nmsgstr ""\n'.format(fn, i+1, quoted_escaped_line))
