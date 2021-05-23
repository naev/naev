#!/usr/bin/python

from glob import glob
from os.path import basename
import re

images  = glob("../dat/gfx/outfit/store/*")
images += glob("../artwork/gfx/outfit/store/*")
images  = list(map( lambda x: basename(x), images ))

imgdict = {}
for i in images:
    imgdict[i] = 0

for file in glob("../dat/outfits/**/*.xml"):
    with open( file, 'r' ) as f:
        m = re.search( "<gfx_store>(.+?)</gfx_store>", f.read() )
        if m:
            s = m.group(1)
            v = imgdict.get(s)
            if not v:
                v = 0
            imgdict[s] = v+1

overused = []
underused = []
for k,v in imgdict.items():
    if "organic" in k:
        continue
    if v > 1:
        overused += [k]
    elif v==0:
        underused += [k]

print( "OVERUSED ASSETS:")
s = sorted(overused, key=lambda x: imgdict[x], reverse=True)
for k in s:
    print( f"   {k}: {imgdict[k]}" )

print( "UNDERUSED ASSETS:")
for k in underused:
    print( f"   {k}" )
