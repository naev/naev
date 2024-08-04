#!/usr/bin/env python3
import json
import yaml
import os
import re
import sys

HEADER = r"""# Naevpedia
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

def needs_translation( line ):
    if not line:
        return False
    ls = line.strip()
    if ls[:2]=="<%" and ls[-2:]=="%>":
        return False
    return True

with open(sys.argv[1],"w") as fout:
    def print_line( fn, i, line ):
        quoted_escaped_line = json.dumps(line, ensure_ascii=False)
        fout.write('#: {}:{}\nmsgid {}\nmsgstr ""\n\n'.format(fn, i+1, quoted_escaped_line))

    fout.write( HEADER )
    for fn in sys.argv[2:]:
        with open( fn, 'r' ) as f:
            fn = re.sub('.*/dat', 'dat', fn)
            d = f.read()
            # Remove meta-data header, TODO parse
            m = d.split('---\n')
            n = 0
            if len(m) > 2:
                y = yaml.safe_load( m[1] )
                n = 2 + len(m[1].splitlines())
                if 'title' in y:
                    print_line( fn, 1, y['title'] )
                d = ''.join(m[2:])
            # Go over lines
            l = d.splitlines()
            for i, line in enumerate(l):
                if needs_translation( line ):
                    print_line( fn, n+i, line )
