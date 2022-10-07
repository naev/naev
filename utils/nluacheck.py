#!/usr/bin/python3
import os
import re
import sys
import pathlib as pl
import argparse
import subprocess

def nluacheck( filename, extra_opts=[] ):
    with open( filename, 'r' ) as f:
        data = f.read()

    hooks = re.findall("hook\.(?!pilot)[a-z]*?\(.*?\"(.+?)\".*?\)", data, re.S)
    pilot_hooks = re.findall("hook\.pilot ?\(.*?,.*?, *?\"(.+?)\".*?\)", data, re.S)
    hooks += pilot_hooks

    hooks = list(set(hooks))
    hooks.sort()

    args = [ "luacheck", filename ]
    args += extra_opts
    for r in hooks:
        args += [ "--globals", r ]
    ret = subprocess.run( args, capture_output=True )

    sys.stdout.buffer.write( ret.stdout )
    if ret.returncode!=0:
        sys.exit( ret.returncode )
    return ret.returncode

if __name__ == "__main__":
    parser = argparse.ArgumentParser( description='Wrapper for luacheck that "understands" Naev hooks.' )
    parser.add_argument('filename', metavar='FILENAME', nargs='+', type=str, help='Name of the file(s) to parse.')
    args, unknown = parser.parse_known_args()

    filelist = set()

    for a in args.filename:
        if a[-4:]==".lua":
            filelist.add( a )
        else:
            p = pl.Path(a)
            for f in p.glob( os.path.join("**", "*.lua") ):
                filelist.add( f )

    filelist = list(filelist)
    filelist.sort()
    for f in filelist:
        nluacheck( f, unknown )
