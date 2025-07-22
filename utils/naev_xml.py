#!/usr/bin/env python3

"""
A slim layer on top of xmltodict. Used as follows:
  o = naev_xml('input.xml')
  # Here o is just the regular xmltodict-generated dictionary, except:

  #  1. the subtrees you insert are deep-copied. This prevents accidentally turning
  #     your tree into a dag. The "downside":
  t = {'a': '1', 'z': '26'}
  o['new_child'] = t
  t['a'] = '2'
  # !!! here o['new_child']['a'] is still '1'

  #  2. You can use an additional '$' character in you tag names:
  o['nums'] = {'fst': '1', 'snd': '2.0', 'thd': '3.14', 'fth': 'zero'}
  # in that case:
  # o['nums']['$fst'] -> 1
  # o['nums']['$snd'] -> 2
  # o['nums']['$thd'] -> 3.14
  # o['nums']['$fth'] -> exception (attempt to use non-value str as a value)
  o['nums']['$fst'] = 1     # saves 1
  o['nums']['$snd'] = 2.0   # saves 2
  o['nums']['$thd'] = 3.14  # saves 3.14
  o['nums']['$fth'] = '1.0' # raises an exception
  # this allows:
  # o['$speed'] *= 1.2
  # o['$thing'] = 2.0 * 0.5 # will save 1 instead of 1.0

  #  3. You get warnings in the following cases:
  #    - the dict gets garbage collected while with unsaved changes.
  #    - the dict gets saved while its destination is already up-to-date.

  # This optional call allows to change the destination.
  # If not called, save destination is the same as input.
  o.save_as('output.xml')
  o.save()
  # xmltodict's unparse was missing a final newline and unable to fold empty
  # tags. This has been fixed.
"""

from xmltodict import *
from sys import stderr
from os import devnull
import re


def _numify( s ):
   try:
      f = float(s)
      return int(f) if round(f) == f else f
   except:
      pass

class _xml_node( dict ):
   def __init__ ( self, mapping, parent = None, key = None ):
      mknode = lambda v: _xml_node(v) if isinstance(v, dict) else v
      dict.__init__(self, {k: mknode(v) for k, v in mapping.items()})
      self._parent = parent
      self._key = key

   def _change( self ):
      if self._parent is None:
         self._uptodate = False
      else:
         self._parent._change()

   def __getitem__ ( self, key ):
      if isinstance(key, str) and key[0] == '$':
         return _numify(dict.__getitem__(self, key[1:]))
      else:
         return dict.__getitem__(self, key)

   def __setitem__( self, key, val ):
      if isinstance(key, str) and key[0] == '$':
         val = _numify(val)
         if val is None:
            raise ValueError(str(val) + ' is not a number.')
         key = key[1:]
      elif isinstance(val, dict) and not isinstance(val, _xml_node):
         val = _xml_node(val, self, key)
      dict.__setitem__(self, key, val)
      self._change()

   def __delitem__ ( self, key ):
      dict.__delitem__(self, key[1:] if key[:1] == '$' else key)

   def parent( self ):
      return self._parent, self._key

   def nodes( self, lookfor = None ):
      for k, v in self.items():
         if not lookfor or k in lookfor:
            if _numify(v) is None:
               yield self, k
            else:
               yield self, '$' + k
         if isinstance(v, _xml_node):
            for t in v.nodes(lookfor):
               yield t

class naev_xml( _xml_node ):
   def __init__( self, fnam = devnull, read_only = False ):
      self._uptodate = True
      if type(fnam) != type('') or (not fnam.endswith('.xml') and fnam != devnull):
         raise Exception('Invalid xml filename ' + repr(fnam))
      self._filename = devnull if read_only else fnam
      self.short = False
      if fnam == devnull:
         _xml_node.__init__(self, {})
      else:
         with open(fnam, 'r') as fp:
            _xml_node.__init__(self, parse(fp.read()))

   def save( self ):
      if self._uptodate:
         stderr.write('Warning: saving unchanged file "' + self._filename + '".\n')
      with open(self._filename, 'w') as fp:
         s = unparse(self, pretty=True, indent=' ')
         s = s.replace('<?xml version="1.0" encoding="utf-8"?>\n', '', 1)
         fp.write(re.sub(r'<([^ ]*)([^>]*)></\1>', r'<\1\2/>', s) + '\n')
      self._uptodate = True

   def save_as( self, filename ):
      self._uptodate = self._uptodate and (filename == self._filename)
      self._filename = filename

   def changed( self ):
      return not self._uptodate

   def __del__( self ):
      if not self._uptodate and self._filename != devnull:
         stderr.write('Warning: unsaved file "' + self._filename + '" at exit.\n')

   def find( self, key, ref = False):
      for d, k in self.nodes(lookfor = { key }):
         return d if ref else d[k]
