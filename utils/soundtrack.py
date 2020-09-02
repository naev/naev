#!/usr/bin/python

import yaml
import zipfile
import glob
import os

from tinytag import TinyTag

with open('dat/snd/soundtrack.yaml','r') as f:
    soundtrack = yaml.safe_load(f)

with open('VERSION', 'r') as f:
    version = f.read().strip()

all_songs = []
for song in glob.glob("dat/snd/music/*.ogg"):
    all_songs.append( os.path.basename(song) )

duplicated_songs = []
with zipfile.ZipFile(f"naev-{version}-soundtrack.zip", 'w', zipfile.ZIP_DEFLATED) as zipf:
    i = 1
    for song in soundtrack:
        if song['filename'] in all_songs:
            all_songs.remove( song['filename'] )
        else:
            duplicated_songs.append( song['filename'] )

        filename = 'dat/snd/music/'+song['filename']
        tag = TinyTag.get(filename)
        length = "%d:%02d" % (tag.duration//60, tag.duration%60)
        number = "%02d" % i
        print( f"{number}. {song['filename']} ({length})" )
        zipf.write( filename, number+"_"+song['filename'] )
        i += 1

if len(all_songs) > 0:
    print( 'MISSING SONGS:' )
    print( all_songs )
if len(duplicated_songs) > 0:
    print( 'DUPLICATED SONGS:' )
    print( duplicated_songs )
