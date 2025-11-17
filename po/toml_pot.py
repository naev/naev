#!/usr/bin/env python3
import os
import sys
import tomllib as toml
import json

HEADER = r"""# Naev toml files
# Copyright (C) YEAR Naev Dev Team
# This file is distributed under the same licence as the naev package.
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

TRANSLATABLES = {
   "damagetype": [
      "name",
      "display",
   ],
   "slots": [
      "display",
      "description",
   ],
}

DATA = {}
def data( filename, string ):
   field = DATA.get(string)
   if field:
      DATA[ string ].append( filename )
   else:
      DATA[ string ] = [ filename ]

dirnames = sys.argv[2:].copy()
dirnames.sort()

for dirname in dirnames:
   for (root, dirs, files) in os.walk( dirname ):
      if not root.endswith('/'):
         root += '/'

      datadir = root.split('/')[-2]
      translates = TRANSLATABLES.get(datadir)
      if not translates:
         print(f"skipping unknown directory {root}")
         continue

      files = list(filter( lambda x: x.endswith(".toml"),files))
      files.sort()
      for filename in files:
         filename = root+filename
         with open( filename, 'rb' ) as f:
            print(f"processing {filename}...")
            d = toml.load( f )

            for t in translates:
               value = d.get(t)
               if value:
                  data( filename, value )

with open(sys.argv[1],"w") as fout:
   fout.write( HEADER )
   for (value,filenames) in DATA.items():
      fout.write('\n')
      for fn in filenames:
         fout.write( f'#: {fn}:1\n')
      fout.write( f'msgid "{value}"\n')
      fout.write( 'msgstr ""\n' )
