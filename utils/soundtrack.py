#!/usr/bin/env python3

import argparse
import glob
import os
import re
import tempfile
import yaml
import zipfile
from concurrent.futures import ThreadPoolExecutor, as_completed

import mutagen


def yaml_list(path):
   try:
      with open(path, 'r') as f:
         return yaml.safe_load(f) or []
   except FileNotFoundError:
      return []


def process_song(snd_dir, song, song_licensing, track_number):
   """
   Process a single song:
   - Read the original file.
   - Modify metadata using mutagen.
   - Save to a temporary file.
   - Return necessary info for zipping and CSV.
   """
   filename = os.path.join(snd_dir, 'music', song['filename'])
   temp = tempfile.NamedTemporaryFile(delete=False, suffix='.ogg').name

   # Copy the original file to temp
   with open(filename, 'rb') as inf, open(temp, 'wb') as outf:
      outf.write(inf.read())

   # Modify metadata
   audio = mutagen.File(temp, easy=True)
   if audio is None:
      print(f"Warning: Could not process metadata for {song['filename']}")
      return None

   audio['title'] = song['name']
   licensing_info = song_licensing.get(song['filename'], {})
   audio['artist'] = licensing_info.get('artist', 'Unknown Artist')
   audio['license'] = licensing_info.get('license', 'Unknown License')
   audio['tracknumber'] = str(track_number).zfill(2)
   audio['organization'] = 'Naev DevTeam'
   audio['album'] = 'Naev Soundtrack'

   audio.save()

   # Get duration
   length_seconds = int(audio.info.length)
   length = f"{length_seconds // 60}:{length_seconds % 60:02d}"

   return {
      'track_number': str(track_number).zfill(2),
      'filename': song['filename'],
      'temp_path': temp,
      'name': song['name'],
      'length': length
   }


def generate_soundtrack(source_dir, output, generate_csv=False, max_workers=None):
   snd_dirs = [os.path.join(source_dir, subdir, 'snd') for subdir in ('dat', 'artwork')]

   # Load licensing information
   song_licensing = {}
   for snd_dir in snd_dirs:
      for author in yaml_list(os.path.join(snd_dir, 'SOUND_LICENSE.yaml')):
         for name, data in author.items():
            license = data.get('license', 'Unknown License')
            songs = data.get('music', [])
            for song in songs:
               song_clean = re.sub(r' \(http[^)]*\)', '', song)  # Strip parenthetical source info
               song_licensing[song_clean] = {'artist': name, 'license': license}

   # Collect all existing songs to check for duplicates and missing songs
   existing_songs = set()
   for snd_dir in snd_dirs:
      for song_path in glob.glob(os.path.join(snd_dir, 'music', '*.ogg')):
         existing_songs.add(os.path.basename(song_path))

   # Collect all songs to process and assign track numbers
   songs_to_process = []
   duplicated_songs = []
   soundtrack_songs = []
   track_number = 1

   for snd_dir in snd_dirs:
      soundtrack = yaml_list(os.path.join(snd_dir, 'soundtrack.yaml'))
      for song in soundtrack:
         soundtrack_songs.append(song)
         if song['filename'] in existing_songs:
            existing_songs.remove(song['filename'])
         else:
            duplicated_songs.append(song['filename'])
         songs_to_process.append((snd_dir, song, track_number))
         track_number += 1

   # Determine the number of workers
   if max_workers is None:
      max_workers = os.cpu_count() or 4  # Default to 4 if os.cpu_count() is None

   # Process songs in parallel
   processed_songs = []
   with ThreadPoolExecutor(max_workers=max_workers) as executor:
      future_to_song = {
         executor.submit(process_song, snd_dir, song, song_licensing, tn): (snd_dir, song)
         for snd_dir, song, tn in songs_to_process
      }
      for future in as_completed(future_to_song):
         result = future.result()
         if result:
            processed_songs.append(result)

   # Sort processed songs by track number to maintain order
   processed_songs.sort(key=lambda x: int(x['track_number']))

   # Create zip file and optionally CSV
   with zipfile.ZipFile(f"{output}.zip", 'w', zipfile.ZIP_DEFLATED) as zipf:
      csvfile = None
      if generate_csv:
         csv_path = f"{output}.csv"
         csvfile = open(csv_path, 'w', encoding='utf-8')
         csvfile.write('"Disc Number","Track Number","Original Name","Original Name Language (ie., ""es"", ""jp"") (optional)","International Name (optional)","Duration (""m:ss"")","ISRC (optional)"\n')

      for song in processed_songs:
         zipf.write(song['temp_path'], f"{song['track_number']}_{song['filename']}")
         print(f"{song['track_number']}. {song['name']} ({song['length']})")

         if generate_csv and csvfile:
            csvfile.write(f"01,{song['track_number']},{song['name']},en,,{song['length']},\n")

         # Remove the temporary file
         os.remove(song['temp_path'])

      if csvfile:
         csvfile.close()

   # Report missing and duplicated songs
   if existing_songs:
      print('MISSING SONGS:')
      print(sorted(existing_songs))
   if duplicated_songs:
      print('DUPLICATED SONGS:')
      print(sorted(duplicated_songs))


if __name__ == "__main__":
   parser = argparse.ArgumentParser(description='Tool to generate the Naev soundtrack.')
   parser.add_argument('--csv', type=bool, default=False,
                  help="Create CSV file that can be used to upload steam metadata.")
   parser.add_argument('--source-dir', default='.', help="Path to Naev's source directory.")
   parser.add_argument('--output', default=None, help="Name of the output file.")
   parser.add_argument('--workers', type=int, default=None, help="Number of parallel workers.")
   args = parser.parse_args()

   output = args.output
   if output is not None:
      if output.endswith('.zip'):
         output = output[:-4]
   else:
      # Get version information
      version_path = os.path.join(args.source_dir, 'dat', 'VERSION')
      if not os.path.exists(version_path):
         print(f"VERSION file not found in {os.path.join(args.source_dir, 'dat')}")
         exit(1)
      with open(version_path, 'r') as f:
         version = f.read().strip()
      output = f"naev-{version}-soundtrack"

   print(f"Generating {output}.zip with "
      f"{args.workers if args.workers else (os.cpu_count() or 4)} workers...")

   generate_soundtrack(
      source_dir=args.source_dir,
      output=output,
      generate_csv=args.csv,
      max_workers=args.workers
   )
