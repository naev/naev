#!/usr/bin/python3

from glob import glob
import os.path
import re

directory = os.path.split(os.getcwd())
if directory[1] == 'utils':
    prefix = '..'
elif directory[1] == 'naev':
    prefix = '.'
else:
    print("Failed to detect where you're running this script from\nPlease enter your path manually")

images  = glob(prefix+"/dat/gfx/outfit/store/*")
images += glob(prefix+"/artwork/gfx/outfit/store/*")
images  = list(map( lambda x: os.path.basename(x), images ))

imgdict = {}
for i in images:
    imgdict[i] = [0, []]

def parse_outfit( file ):
    print( file )
    with open( file, 'r' ) as f:
        d = f.read()
        m = re.search( "<gfx_store>(.+?)</gfx_store>", d )
        if m:
            s = m.group(1)
            v = imgdict.get(s)
            if not v:
                v = [ 0, [] ]
            v[0] += 1
            v[1] += [os.path.basename(file)]
            imgdict[s] = v

for file in glob(prefix+"/dat/outfits/**/*.xml", recursive=True):
    parse_outfit( file )
for file in glob(prefix+"/build/dat/outfits/**/*.xml", recursive=True):
    parse_outfit( file )

overused = []
underused = []
for k,v in imgdict.items():
    if "organic" in k:
        continue
    if v[0] > 1:
        overused += [k]
    elif v[0]==0:
        underused += [k]

with open( "outfit_gfx.html", "w" ) as out:
    out.write( """
<html>
<head>
 <title>Naev Outfit Graphic Used Status</title>
</head>
<body>
    """ )
    for k in sorted( imgdict, key=lambda x: imgdict[x], reverse=True ):
        path = prefix+"/dat/gfx/outfit/store/"+k
        if not os.path.isfile( path ):
           path = prefix+"/artwork/gfx/outfit/store/"+k
        if not os.path.isfile( path ):
           path = prefix+"/artwork/"+k
        v = imgdict[k]
        if v[0] != 1: # for simplicity hide stuff that appears once
            out.write(f"""
  <div>
   <img width="128" height="128" src='{path}' />
   <span>{k}: {v[0]}</span><br/>
   <span>{', '.join(v[1])}</span>
  </div>
""")
    out.write( """
</body>
</html>
    """ )

print( "OVERUSED ASSETS:")
s = sorted(overused, key=lambda x: imgdict[x][0], reverse=True)
for k in s:
    print( f"   {k}: {imgdict[k][0]} ({', '.join(imgdict[k][1])})" )

print( "UNDERUSED ASSETS:")
for k in underused:
    print( f"   {k}" )
