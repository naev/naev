#!/usr/bin/env python3

from os import path, utime
from sys import stderr
from pathlib import Path
from outfit import outfit

from xmllua2mvx import xmllua2mvx 
from mvx2xmllua import mvx2xmllua

def mvx_nam(xml):
   return path.join(path.dirname(xml), '.' + path.basename(xml)[:-3] + 'mvx')

def _gen_if_needed( xml, force = False ):
   mvx = mvx_nam(xml)
   exists = (not force) and Path(mvx).is_file()
   uptodate = exists and not path.getmtime(mvx) < path.getmtime(xml)
   if not uptodate:
      return xmllua2mvx(xml, mvx, quiet = False)
   else:
      return outfit(mvx)

def core_outfit( nam, try_again = False ):
   if nam[-4:] == '.xml':
      try:
         o = _gen_if_needed(nam)
      except:
         o = None
      if o is None and try_again:
         o = _gen_if_needed(nam, True)
      if not (o is None):
         o.fil = nam
      return o

def some_outfit( nam ):
   o = core_outfit(nam)
   return o if not (o is None) else outfit(nam)

def core_write( o, fil ):
   mvx = mvx_nam(fil)
   o.write(mvx)
   mvx2xmllua(mvx, fil, quiet = True)
   # mark mvx as up to date
   with open(mvx, 'ab'):
      utime(mvx, None)
