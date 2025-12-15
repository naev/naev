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
   "damagetype": {
      "fields": [
         "name",
         "display",
      ],
      "context": "damagetype",

   },
   "slots": {
      'fields': [
         "display",
         "description",
      ],
      "context": "slotproperty",
   },
   "start.toml": {
      'fields': [
         "ship_name",
         "ship_acquired",
         "scenario_name",
      ],
   },
}

DATA = {}
def data( filename, string, context=None ):
   if not context:
      context = "NONE"
   if not DATA.get(context):
      DATA[ context ] = {}
   if not DATA[ context ].get(string):
      DATA[ context ][ string ] = []
   DATA[ context ][ string ].append( filename )

dirnames = sys.argv[2:].copy()
dirnames.sort()

for dirname in dirnames:
   if os.path.isfile(dirname):
      datafile = dirname.split('/')[-1]
      translates = TRANSLATABLES.get(datafile)

      with open( dirname, 'rb' ) as f:
         print(f"processing {dirname}...")
         d = toml.load( f )

         for t in translates["fields"]:
            value = d.get(t)
            if value:
               data( dirname, value, translates.get("context") )

   else:
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
               #print(f"processing {filename}...")
               d = toml.load( f )

               for t in translates["fields"]:
                  value = d.get(t)
                  if value:
                     data( filename, value, translates.get("context") )

out = HEADER
for (context,datas) in DATA.items():
   for (value,filenames) in datas.items():
      out += '\n'
      for fn in filenames:
         out += f'#: {fn}:1\n'
      if context != "NONE":
         out += f'msgctxt "{context}"\n'
      out += f'msgid "{value}"\n'
      out += 'msgstr ""\n'

try:
   with open(sys.argv[1],"r") as fin:
      upstream = fin.read()
except:
   upstream = ""

if out != upstream:
   with open(sys.argv[1],"w") as fout:
      fout.write( out )
