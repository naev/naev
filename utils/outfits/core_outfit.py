#!/usr/bin/env python3

from os import path
from sys import stderr
from pathlib import Path
from outfit import outfit
import subprocess

script = path.join(path.dirname(__file__), 'xmllua2mvx.py')

def _gen_if_needed( xml, force = False ):
   mvx = path.join(path.dirname(xml), '.' + path.basename(xml[:-3] + 'mvx'))
   exists = (not force) and Path(mvx).is_file()
   uptodate = exists and not path.getmtime(mvx) < path.getmtime(xml)
   if not uptodate:
      stderr.write(('upd' if exists else 'gener') + 'ate "' + path.basename(mvx) + '"\n')
      res = subprocess.run([script, xml], capture_output = True).stdout.decode()
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
   else:
      o = outfit(nam)
   return o
