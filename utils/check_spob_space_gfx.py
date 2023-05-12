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

images  = glob(prefix+"/artwork/gfx/spob/space/*")
images  = list(map( lambda x: os.path.basename(x), images ))

imgdict = {}
for i in images:
    imgdict[i] = [0, []]

for file in glob(prefix+"/dat/spob/**/*.xml", recursive=True):
    print( file )
    with open( file, 'r' ) as f:
        d = f.read()

        m = re.search( "<space>(.+?)</space>", d )
        if m:
            s = m.group(1)
            v = imgdict.get(s)
            if not v:
                v = [ 0, [] ]
            v[0] += 1
            v[1] += [os.path.basename(file)]
            imgdict[s] = v

overused = []
underused = []
for k,v in imgdict.items():
    if v[0] > 1:
        overused += [k]
    elif v[0]==0:
        underused += [k]

with open( "spob_gfx.html", "w" ) as out:
    out.write( """
<html>
<head>
 <title>Naev Spob Graphic Used Status</title>
</head>
<body>
    """ )
    for k in sorted( imgdict, key=lambda x: imgdict[x], reverse=True ):
        path = prefix+"/artwork/gfx/spob/space/"+k
        v = imgdict[k]
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
underused.sort()
for k in underused:
    print( f"   {k}" )
