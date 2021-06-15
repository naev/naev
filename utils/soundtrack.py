#!/usr/bin/env python3

import argparse
import glob
import os
import tempfile
import yaml
import zipfile

import mutagen


def yaml_list(path):
    try:
        return yaml.safe_load(open(path))
    except FileNotFoundError:
        return []


def generate_soundtrack( source_dir, output, generate_csv=False ):
    snd_dirs = [os.path.join(source_dir, subdir, 'snd') for subdir in ('dat', 'artwork')]
    # Load licensing information
    song_licensing = {}
    for snd_dir in snd_dirs:
        for author in yaml_list(os.path.join(snd_dir, 'SOUND_LICENSE.yaml')):
            (name, data), = author.items()
            license = data['license']
            songs = data.get('music', [])
            for song in songs:
                song_licensing[song] = {'artist':name,'license':license}

    # Get all existing songs to check for duplicates and missing songs
    all_songs = []
    for snd_dir in snd_dirs:
        for song in glob.glob(os.path.join(snd_dir, 'music', '*.ogg')):
            all_songs.append( os.path.basename(song) )

    # Run over the soundtrack to create it as a zipfile
    duplicated_songs = []
    with zipfile.ZipFile(f"{output}.zip", 'w', zipfile.ZIP_DEFLATED) as zipf:
        i = 1
        if generate_csv:
            csvfile = open( f"{output}.csv", 'w' )
            csvfile.write( '"Disc Number","Track Number","Original Name","Original Name Language (ie., ""es"", ""jp"") (optional)","International Name (optional)","Duration (""m:ss"")","ISRC (optional)"\n' )
        for snd_dir in snd_dirs:
            soundtrack = yaml_list(os.path.join(snd_dir, 'soundtrack.yaml'))
            for song in soundtrack:
                if song['filename'] in all_songs:
                    all_songs.remove( song['filename'] )
                else:
                    duplicated_songs.append( song['filename'] )

                filename = os.path.join(snd_dir, 'music', song['filename'])
                temp = tempfile.NamedTemporaryFile().name
                with open( temp, 'wb' ) as outf:
                    with open( filename, 'rb' ) as inf:
                        outf.write( inf.read() )
                audio = mutagen.File( temp )
                length = "%d:%02d" % (audio.info.length//60, audio.info.length%60)
                number = "%02d" % i
                # Set up metadata
                audio['TITLE']      = song['name']
                licensing_info      = song_licensing[song['filename']]
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
    parser.add_argument('--source-dir', default='.', help="Path to Naev's source directory.")
    parser.add_argument('--output', default=None, help="Name of the output file.")
    args = parser.parse_args()

    output = args.output
    if output is not None:
        if output.endswith('.zip'):
            output = output[0:-4]
    else:
        # Get version information
        with open(os.path.join(args.source_dir, 'dat/VERSION'), 'r') as f:
            version = f.read().strip()
        output = f"naev-{version}-soundtrack"

    print(f"Generating {output}.zip")

    generate_soundtrack( source_dir=args.source_dir, output=output, generate_csv=args.csv )
