#!/usr/bin/python3
import os
import re
import sys
import pathlib as pl
import argparse
import subprocess
from multiprocessing import Pool

HOOKS_REGEX = re.compile( r'hook\.(?!pilot)[a-z]*\s*\(.*?"(.+?)"' )
HOOKS_PILOT_REGEX = re.compile( r'hook\.pilot\s*\(.*?,.*?, *?"(.+?)"' )

def nluacheck( filename, extra_opts=[] ):
    with open( filename, 'r', encoding='utf-8' ) as f:
        data = f.read()

    # XXX - will not detect multi-line calls unless the function name is on the first line.
    hooks = HOOKS_REGEX.findall( data )
    pilot_hooks = HOOKS_PILOT_REGEX.findall( data )
    hooks += pilot_hooks

    hooks = sorted(set(hooks))

    args = [ "luacheck", filename ]
    args += extra_opts
    for r in hooks:
        args += [ "--globals", r ]
    ret = subprocess.run( args, capture_output=True )

    return ret.returncode, ret.stdout

if __name__ == "__main__":
    parser = argparse.ArgumentParser( description='Wrapper for luacheck that "understands" Naev hooks.' )
    parser.add_argument('filename', metavar='FILENAME', nargs='+', type=str, help='Name of the file(s) to parse.')
    parser.add_argument('-j', '--jobs', metavar='jobs', type=int, default=None, help='Number of jobs to use. Defaults to number of CPUs.')
    args, unknown = parser.parse_known_args()

    filelist = set()

    for a in args.filename:
        if a[-4:]==".lua":
            filelist.add( a )
        else:
            p = pl.Path(a)
            for f in p.glob( os.path.join("**", "*.lua") ):
                filelist.add( f )

    def nluacheck_w( filename ):
        return nluacheck( filename, unknown )

    filelist = list(filelist)
    filelist.sort()
    with Pool( args.jobs ) as pool:
        retlist = pool.map( nluacheck_w, filelist )
    err = 0
    for r in retlist:
        sys.stdout.buffer.write( r[1] )
        if r[0]!=0:
            err = r[0]
    sys.exit( err )
