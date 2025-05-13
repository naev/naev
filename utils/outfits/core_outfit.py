#!/usr/bin/env python3

from os import path, utime
from sys import stderr
from pathlib import Path
from outfit import outfit
import subprocess

script_dir = path.dirname(__file__)
xml2mvx = path.join(script_dir, 'xmllua2mvx.py')
mvx2xml = path.join(script_dir, 'mvx2xmllua.py')

def mvx_nam(xml):
   return path.join(path.dirname(xml), '.' + path.basename(xml)[:-3] + 'mvx')

def _gen_if_needed( xml, force = False ):
   mvx = mvx_nam(xml)
   exists = (not force) and Path(mvx).is_file()
   uptodate = exists and not path.getmtime(mvx) < path.getmtime(xml)
   if not uptodate:
      res = subprocess.run([xml2mvx, xml], capture_output = True).stdout.decode()
      if res == '':
         return None
      stderr.write(('upd' if exists else 'gen') + ' "' + path.basename(mvx) + '"\n')
      with open(mvx, "w") as fp:
         fp.write(res)
   else:
      with open(mvx, "r") as fp:
         res = fp.read()
      if res == '':
         return None
   return res

def core_outfit( nam, try_again = False ):
   if nam[-4:] == '.xml':
      try:
         o = outfit(_gen_if_needed(nam), content = True)
      except:
         o = None
      if o is None and try_again:
         o = outfit(_gen_if_needed(nam, True), content = True)
      if not (o is None):
         o.fil = nam
         return o

def some_outfit( nam ):
   o = core_outfit(nam)
   return o if not (o is None) else outfit(nam)

def core_write( o, fil ):
   mvx = mvx_nam(fil)
   o.write(mvx)
   subprocess.run([mvx2xml, '-q', mvx, fil])
   # mark mvx as up to date
   with open(mvx, 'ab'):
      utime(mvx, None)
