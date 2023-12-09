#!/usr/bin/python
import os
import sys
import polib
import subprocess
import tempfile
import language_tool_python

# Below is a chunk from an example vimrc of getting this to work with vim
"""
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
"""

assert(len(sys.argv)>1)

os.environ["LC_ALL"] = "C"

filename = sys.argv[1]
if filename=='--autoDetect':
    filename = sys.argv[2]

with open( filename, 'r' ) as f:
    data = f.read()
lines = data.splitlines()

tf = tempfile.NamedTemporaryFile( suffix='.po' )
args = [ "xgettext", "-", '--language=lua', '-d' , tf.name[:-3] ]
ret = subprocess.run( args, input=bytes(data,'utf-8') )

# If we use 'en-GB' it gets a ton of false positives, not sure how to solve it...
#tool = language_tool_python.LanguageTool('en-GB')
tool = language_tool_python.LanguageTool('en')
po = polib.pofile( tf.name )
n = 1
for entry in po:
    clist = tool.check( entry.msgid )
    for c in clist:
        line = int(entry.occurrences[1][1])
        f = lines[line-1].find(entry.msgid)
        assert( f>= 0)
        col = 1+f+c.offset
        print(f"{n}.) Line {line}, column {col}, Rule ID: {c.ruleId} premium: false")
        print(f"Message: {c.message}")
        print(f"{c.context}")
        print(f"{' '*c.offsetInContext}{'^'*c.errorLength}")
        print()
        n = n+1
