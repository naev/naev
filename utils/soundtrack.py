#!/usr/bin/python3

import yaml
import zipfile
import glob
import os
import tempfile
import argparse

import mutagen


def generate_soundtrack( generate_csv=False ):

    # Load licensing information
    with open('dat/snd/SOUND_LICENSE.yaml','r') as f:
        sound_license = yaml.safe_load(f)
    song_licensing = {}
    for author in sound_license:
        name = list(author.keys())[0]
        data = author[name]
        license = data['license']
        songs = data.get('music')
        if songs == None:
            continue
        for song in songs:
            song_licensing[song] = {'artist':name,'license':license}

    # Get version information
    with open('VERSION', 'r') as f:
        version = f.read().strip()

    # Get all existing songs to check for duplicates and missing songs
    all_songs = []
    for song in glob.glob("dat/snd/music/*.ogg"):
        all_songs.append( os.path.basename(song) )

    # Get the soundtrack information
    with open('dat/snd/soundtrack.yaml','r') as f:
        soundtrack = yaml.safe_load(f)

    # Run over the soundtrack to create it as a zipfile
    duplicated_songs = []
    with zipfile.ZipFile(f"naev-{version}-soundtrack.zip", 'w', zipfile.ZIP_DEFLATED) as zipf:
        i = 1
        if generate_csv:
            csvfile = open( f"naev-{version}-soundtrack.csv", 'w' )
            csvfile.write( '"Disc Number","Track Number","Original Name","Original Name Language (ie., ""es"", ""jp"") (optional)","International Name (optional)","Duration (""m:ss"")","ISRC (optional)"\n' )
        for song in soundtrack:
            if song['filename'] in all_songs:
                all_songs.remove( song['filename'] )
            else:
                duplicated_songs.append( song['filename'] )

            filename = 'dat/snd/music/'+song['filename']
            temp = tempfile.NamedTemporaryFile().name
            with open( temp, 'wb' ) as outf:
                with open( filename, 'rb' ) as inf:
                    outf.write( inf.read() )
            audio = mutagen.File( temp )
            length = "%d:%02d" % (audio.info.length//60, audio.info.length%60)
            number = "%02d" % i
            # Set up metadata
            audio['TITLE']      = song['name']
            licensing_info      = song_licensing.get(song['filename'])
            audio['ARTIST']     = licensing_info['artist']
            audio['LICENSE']    = licensing_info['license']
            audio['TRACKNUMBER'] = number
            audio['ORGANIZATION'] = 'Naev DevTeam'
            audio['ALBUM']      = 'Naev Soundtrack'
            print( f"{number}. {song['name']} ({length})" )
            # Write to zip
            audio.save()
            zipf.write( temp, number+"_"+song['filename'] )
            os.remove( temp )

            if generate_csv:
                csvfile.write( f"01,{number},{song['name']},en,,{length},\n" )
            i += 1
        if generate_csv:
            csvfile.close()

    if len(all_songs) > 0:
        print( 'MISSING SONGS:' )
        print( all_songs )
    if len(duplicated_songs) > 0:
        print( 'DUPLICATED SONGS:' )
        print( duplicated_songs )


if __name__=="__main__":
    parser = argparse.ArgumentParser(description='Tool generate the Naev soundtrack.')
    parser.add_argument('--csv', type=bool, default=False, help="Create CSV file that con be used to upload steam metadata.")
    args = parser.parse_args()
    generate_soundtrack( generate_csv=args.csv )


