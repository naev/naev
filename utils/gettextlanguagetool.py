#!/usr/bin/python
import os
import sys
import polib
import subprocess
import tempfile
import language_tool_python

"""
This scripts extracts all the gettext translatable fields from a file and then
runs languagetool on each of the outputs. The output follows the same format as
languagetool while keeping the line numbers and offsets relative to the
original input file.

Below is a chunk from an example vimrc of getting this to work with vim
```
" Set up ALE plugin used plugged
call plug#begin('~/.vim/plugged')
 Plug 'dense-analysis/ale'
call plug#end()

" Create the Lua linter
call ale#handlers#languagetool#DefineLinter('lua')
" Set the tool path to the custom tool, should be wherever you have it
let g:ale_languagetool_executable = "/PATH/TO/naev/utils/gettextlint.py"

" Enable the linter by adding it to the defaults
let g:ale_linters = {
\  'lua':['languagetool', 'cspell', 'lua_language_server', 'luac', 'luacheck', 'selene'],
\}
```
"""

assert(len(sys.argv)>1)

os.environ["LC_ALL"] = "C"

# TODO better handling parameters and ideally passing them on to xgettext
filename = sys.argv[1]
if filename=='--autoDetect':
    filename = sys.argv[2]

with open( filename, 'r' ) as f:
    lines = f.read().splitlines()

tf = tempfile.NamedTemporaryFile( suffix='.po' )
args = [ "xgettext", filename, '--from-code=utf-8', '-d' , tf.name[:-3] ] # xgettext adds .po again
ret = subprocess.run( args )

# TODO implement a custom dictionary or something to lower false positives
tool = language_tool_python.LanguageTool('en-GB')
po = polib.pofile( tf.name )
n = 1
for entry in po:
    for ti,txt in enumerate(entry.msgid.splitlines()):
        line = int(entry.occurrences[0][1])+ti
        f = lines[line-1].find(txt)
        assert( f>= 0)
        clist = tool.check(txt)
        for c in clist:
            col = 1+f+c.offset
            print(f"{n}.) Line {line}, column {col}, Rule ID: {c.ruleId}")
            print(f"Message: {c.message}")
            print(f"{c.context}")
            print(f"{' '*c.offsetInContext}{'^'*c.errorLength}")
            print()
            n = n+1
