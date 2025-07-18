#!/usr/bin/env python3

from xmltodict import *
from sys import stderr

_headless = lambda s: s.replace('<?xml version="1.0" encoding="utf-8"?>\n', '', 1)
intify = lambda x: int(x) if x == round(x) else x

class _outfit_node( dict ):
   def __init__ ( self, mapping, parent = None ):
      mknode = lambda v: _outfit_node(v) if isinstance(v, dict) else v
      dict.__init__(self, {k: mknode(v) for k, v in mapping.items()})
      self._parent = parent

   def _changed( self ):
      if self._parent is None:
         self._uptodate = False
      else:
         self._parent._changed()

   def __getitem__ (self, key):
      if isinstance(key, str) and key[0]=='$':
         val = intify(float(dict.__getitem__(self, key[1:])))
      else:
         val = dict.__getitem__(self, key)
      return val

   def __setitem__(self, key, val):
      if isinstance(key, str) and key[0]=='$':
         key, val = key[1:], intify(val)
      elif isinstance(val, dict) and not isinstance(val, _outfit_node):
         val = _outfit_node(val, self)
      dict.__setitem__(self, key, val)
      self._changed()

class xml_outfit( _outfit_node ):
   def __init__( self, fnam ):
      self._uptodate = False
      selfi_.filename = fnam
      with open(fnam, 'r') as fp:
         _outfit_node.__init__(self, parse(fp.read()))

   def save( self ):
      if self._uptodate:
         stderr.write('Warning: saving unchanged file "' + self.filename + '".\n')
      with open(self._filename, 'w') as fp:
         fp.write(_headless(unparse(self, pretty=True, indent=1)) + '\n')
      self._uptodate = True

   def save_as( self, filename ):
      self._uptodate = self._uptodate and (filename == self.filename)
      self._filename = filename

   def __del__( self ):
      if not self.uptodate:
         stderr.write('Warning: unsaved file "' + self.filename + '" at exit.\n')
