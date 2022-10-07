#!/usr/bin/python3
import os
import re
import sys
import pathlib as pl
import argparse
import subprocess

def nluacheck( filename ):
    with open( filename, 'r' ) as f:
        data = f.read()

    result = re.findall("hook\.[a-z]*?\(.*?\"(.+?)\".*?\)", data, re.S)
    result = list(set(result))
    result.sort()

    args = [ "luacheck", filename ]
    for r in result:
        args += [ "--globals", r ]
    ret = subprocess.run( args, capture_output=True )

    sys.stdout.buffer.write( ret.stdout )
    if ret.returncode!=0:
        sys.exit( ret.returncode )
    return ret.returncode

if __name__ == "__main__":
    parser = argparse.ArgumentParser( description='Wrapper for luacheck that "understands" Naev hooks.' )
    parser.add_argument('filename', metavar='FILENAME', nargs='+', type=str, help='Name of the file(s) to parse.')
    args = parser.parse_args()

    filelist = set()

    for a in args.filename:
        if a[-4:]==".lua":
            filelist.add( a )
        else:
            p = pl.Path(a)
            for f in p.glob( os.path.join("**", "*.lua" ) ):
                filelist.add( f )

    filelist = list(filelist)
    filelist.sort()
    for f in filelist:
        nluacheck( f )
