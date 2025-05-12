#!/usr/bin/env python3

from os import path
from sys import stderr
from pathlib import Path
from outfit import outfit
import subprocess

script_dir = path.dirname(__file__)
xml2mvx = path.join(script_dir , 'xmllua2mvx.py')
mvx2xml = path.join( script_dir, 'mvx2xmllua.py')

def mvx_nam(xml):
   return path.join(path.dirname(xml), '.' + path.basename(xml)[:-3] + 'mvx')

def _gen_if_needed( xml, force = False ):
   mvx = mvx_nam(xml)
   exists = (not force) and Path(mvx).is_file()
   uptodate = exists and not path.getmtime(mvx) < path.getmtime(xml)
   if not uptodate:
      stderr.write(('upd' if exists else 'gener') + 'ate "' + path.basename(mvx) + '"\n')
      res = subprocess.run([xml2mvx, xml], capture_output = True).stdout.decode()
      with open(mvx, "w") as fp:
         fp.write(res)
   else:
      with open(mvx, "r") as fp:
         res = fp.read()
   return res

def core_outfit( nam ):
   if nam[-4:] == '.xml':
      try:
         o = outfit(_gen_if_needed(nam), content = True)
      except:  # file was deleted while reading, gen a new one
         o = outfit(_gen_if_needed(nam, True), content = True)
      o.fil = nam
      return o

def some_outfit( nam ):
   return core_outfit(nam) or outfit(nam)

def core_write( fil ):
   mvx = mvx_nam(fil)
   subprocess.run([mvx2xml, '-q', fil, mvx])
