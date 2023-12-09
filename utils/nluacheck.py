#!/usr/bin/python3
import os
import re
import sys
import pathlib as pl
import argparse
import subprocess
from multiprocessing import Pool

# Can also be used with vim by adding the following lines to your vimrc
"""
" Set up ALE plugin used plugged
call plug#begin('~/.vim/plugged')
 Plug 'dense-analysis/ale'
call plug#end()
" ALE Lua settingsshould use luacheck by default
" Remember to change the below paths to your Naev directory
let g:ale_lua_luacheck_executable = '/PATH/TO/naev/utils/nluacheck.py'
let g:ale_lua_luacheck_options = '--config /PATH/TO/naev/.luacheckrc'
"""

# TODO also filter out hook.custom from HOOKS_REGEX
HOOKS_REGEX = re.compile( r'hook\.(?!pilot)[_a-z]*\s*\(.*?"(.+?)"' )
HOOKS_PILOT_REGEX = re.compile( r'hook\.pilot\s*\(.*?,.*?, *?"(.+?)"' )
HOOKS_CUSTOM_REGEX = re.compile( r'hook\.custom\s*\(.*?, *?"(.+?)"' )
NPC_REGEX = re.compile( r'(evt|misn)\.npcAdd\( *?"(.+?)"' )
AI_REGEX = re.compile( r'ai.pushtask\( *?"(.+?)"' )
PROXIMITY_REGEX = re.compile( r'hook\.timer\(.*?, "(proximity|proximityScan|invProximity)", *?{.*?funcname *?= *?"(.+?)"' )

stdin = sys.stdin.read()

def nluacheck( filename, extra_opts=[] ):
    isstdin = False
    if filename=='-':
        data = stdin
        isstdin = True
    else:
        with open( filename, 'r', encoding='utf-8' ) as f:
            data = f.read()

    # XXX - will not detect multi-line calls unless the function name is on the first line.
    hooks = HOOKS_REGEX.findall( data )
    hooks += HOOKS_PILOT_REGEX.findall( data )
    hooks += HOOKS_CUSTOM_REGEX.findall( data )
    hooks += list(map( lambda x: x[1], NPC_REGEX.findall( data ) ) )
    hooks += AI_REGEX.findall( data )
    hooks += list(map( lambda x: x[1], PROXIMITY_REGEX.findall( data ) ) )
    hooks = sorted(set(hooks)) # remove duplicates

    args = [ "luacheck" ]
    args += [ filename ]
    args += extra_opts
    for r in hooks:
        args += [ "--globals", r ]
    if isstdin:
        ret = subprocess.run( args, capture_output=True, input=bytes(data,'utf-8') )
    else:
        ret = subprocess.run( args, capture_output=True )

    return ret.returncode, ret.stdout

if __name__ == "__main__":
    with open( "/tmp/foobar", "w" ) as f:
        f.write(' '.join(sys.argv))
    parser = argparse.ArgumentParser( description='Wrapper for luacheck that "understands" Naev hooks.' )
    parser.add_argument('path', metavar='PATH', nargs='+', type=str, help='Name of the path(s) to parse. Recurses over .lua files in the case of directories.')
    parser.add_argument('-j', '--jobs', metavar='jobs', type=int, default=None, help='Number of jobs to use. Defaults to number of CPUs.')
    # Below stuff for compatibility
    parser.add_argument('--filename', type=str, default=None )
    parser.add_argument('--formatter', type=str, default=None )
    parser.add_argument('--config', type=str, default=None )
    args, unknown = parser.parse_known_args()

    # find files, either as directories or direct names based on extension
    filelist = set()
    for a in args.path:
        if a=='-':
            filelist.add( a )
        elif os.path.isfile(a):
            filelist.add( a )
        elif os.path.isdir(a):
            p = pl.Path(a)
            for f in p.glob( os.path.join("**", "*.lua") ):
                filelist.add( f )
        else:
            parser.print_help()
            sys.stderr.write(f"\nError: non-existant path '{a}'\n")
            sys.exit(-1)

    if args.filename:
        unknown += ["--filename", args.filename]
    if args.formatter:
        unknown += ["--formatter", args.formatter]
    if args.config:
        unknown += ["--config", args.config]

    # Wrapper that will pass through unknown parameters meant for luacheck
    def nluacheck_w( filename ):
        return nluacheck( filename, unknown )

    if filelist=={'-'}:
        retlist = [ nluacheck_w( '-' ) ]
    else:
        filelist = list(filelist)
        filelist.sort()
        with Pool( args.jobs ) as pool:
            retlist = pool.map( nluacheck_w, filelist )
    err = 0
    for r in retlist:
        if r[0]!=0:
            err = r[0]
            # only write to stdeout in class of error for less spam
            sys.stdout.buffer.write( r[1] )
    sys.exit( err )
