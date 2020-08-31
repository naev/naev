#!/usr/bin/python

import yaml
import zipfile

from tinytag import TinyTag

with open('dat/snd/soundtrack.yaml','r') as f:
    soundtrack = yaml.safe_load(f)

with open('VERSION', 'r') as f:
    version = f.read().strip()

with zipfile.ZipFile(f"naev-{version}-soundtrack.zip", 'w', zipfile.ZIP_DEFLATED) as zipf:
    i = 1
    for song in soundtrack:
        filename = 'dat/snd/music/'+song['filename']
        tag = TinyTag.get(filename)
        length = "%d:%02d" % (tag.duration//60, tag.duration%60)
        number = "%02d" % i
        print( f"{number}. {song['filename']} ({length})" )
        zipf.write( filename, number+"_"+song['filename'] )
        i += 1
