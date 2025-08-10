#!/usr/bin/env python3

#TODO: introduce new list wrapper to fix list breach
"""
An elementTree-based implementation of xmltodict. 10% slower than pure elementTree,
but 2x faster than xmltodict with more functionality and correct output
(empty element folding, new lines, no useless xml 1.0 header, etc.).

Used as follows:
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

  #  3. At init, you can specify the r and w flags (both True by default).
  #     You get errors in the following cases:
  #      - r is True and the input file can't be openened
  #      - w is False and you attempt to save
  #     You get warnings in the following cases:
  #      - w is true the dict gets garbage collected while with unsaved changes.
  #      - the dict gets saved while its destination is already up-to-date.

  # This optional call allows to change the destination.
  # If not called, save destination is the same as input.
  o.save_as('output.xml')
  o.save()
  # xmltodict's unparse was missing a final newline and unable to fold empty
  # tags. This has been fixed.
"""

import xml.etree.ElementTree as ET
from sys import stderr, stdout
from os import devnull

def _numify( s ):
   try:
      f = float(s)
      return int(f) if round(f) == f else f
   except:
      pass

class xml_node( dict ):
   def __init__ ( self, mapping, parent= None, key= None ):
      self._parent = parent
      self._key = key
      if isinstance(mapping, xml_node):
         self.attr = mapping.attr.copy()
      else:
         self.attr = {}
      for k, v in mapping.items():
         self[k] = v

   def _change( self ):
      if self._parent is None:
         self._uptodate = False
      else:
         self._parent._change()

   def tag( self ):
      return self._key

   def parent( self ):
      return self._parent

   def __contains__ ( self, key ):
      key = key[1:] if key[:1] == '$' else key
      return key in self.attr if key[:1] == '@' else dict.__contains__(self, key)

   def __getitem( self, key ):
      return self.attr[key] if key[:1] == '@' else dict.__getitem__(self, key)

   def __getitem__( self, key ):
      if isinstance(key, str) and key[0] == '$':
         return _numify(self.__getitem(key[1:]))
      else:
         return self.__getitem(key)

   def __setitem__( self, key, val ):
      if isinstance(key, str) and key[0] == '$':
         val = _numify(val)
         if val is None:
            raise ValueError(str(val) + ' is not a number.')
         key = key[1:]
      elif isinstance(val, list):
         val = [xml_node(e, self, key) if isinstance(e, dict) else e for e in val]
      elif isinstance(val, dict):
         val = xml_node(val, self, key)
      if key[:1] == '@':
         self.attr[key] = val
      else:
         dict.__setitem__(self, key, val)
      self._change()

   def __delitem__( self, key ):
      if key[:1] == '@':
         del self.attr[key]
      else:
         dict.__delitem__(self, key)

   def __repr__( self ):
      return dict.__repr__(self.attr | self)

   __str__ = __repr__

class _trusted_node( xml_node ):
   def __init__ ( self, attr, parent= None, key= None ):
      self.attr = {('@' + k): v for k, v in attr.items()}
      dict.__init__(self, {})
      self._parent = parent
      self._key = key

def _parse( node, par, key ):
   d = _trusted_node(node.attrib, par, key)

   for e in node:
      if dict.__contains__(d, e.tag):
         if not isinstance(d[e.tag], list):
            dict.__setitem__(d, e.tag, [dict.__getitem__(d, e.tag)])
         dict.__setitem__(d, e.tag, dict.__getitem__(d, e.tag) + [_parse(e, d, e.tag)])
      else:
         dict.__setitem__(d, e.tag, _parse(e, d, e.tag))

   t = node.text
   t = t and t.strip() or None
   if d == {} and d.attr == {}:
      return t
   if t:
      dict.__setitem__(d, '#text', t)
   return d

def _unparse_elt( v, k, indent):
   out = indent*' ' + '<' + k
   content = ''

   if isinstance(v, xml_node):
      for ka, va in v.attr.items():
         out += ' ' + ka[1:] + '="' + str(va).replace('&', '&amp;') + '"'
      if dict.__contains__(v, '#text'):
         content = v['#text']
      elif sub := unparse(v, indent+1):
         content = '\n' + sub + indent*' '
   elif v or v == 0:
      content = str(v).replace('&', '&amp;')

   if content:
      out += '>' + content + '</' + k + '>\n'
   else:
      out += '/>\n'
   return out

def unparse( d , indent= 0 ):
   out = ''
   for k, v in d.items():
      if not isinstance(v, list):
         v = [v]
      out += ''.join((_unparse_elt(ve, k, indent) for ve in v))
   return out

class naev_xml( xml_node ):
   def __init__( self, fnam= None, r= True, w= True):
      fnam = fnam or devnull
      self._uptodate = True
      self.w, self.r = w, r
      if type(fnam) != type('') or (not fnam.endswith('.xml') and fnam != devnull):
         raise Exception('Invalid xml filename ' + repr(fnam))
      self._filename = fnam

      self.short = False
      _trusted_node.__init__(self, {}, None, None)
      if r and self._filename != devnull:
         try:
            T = ET.parse(self._filename).getroot()
            dict.__setitem__(self, T.tag, _parse(T, self, T.tag))
         except FileNotFoundError:
            raise FileNotFoundError
         except:
            raise Exception('Invalid xml file "' + repr(self._filename) + '"')

   def save( self, if_needed= False ):
      if not self.w:
         raise Exception('Attempt to save read-only "' + repr(self._filenam) + '"')
      if self._uptodate:
         if if_needed:
            return False
         else:
            stderr.write('Warning: saving unchanged file "' + self._filename + '".\n')

      if self._filename == '-':
         stdout.write(unparse(self))
      else:
         with open(self._filename, 'w') as fp:
            fp.write(unparse(self))

      self._uptodate = True
      return True

   def save_as( self, filename= None ):
      filename = filename or devnull
      if not self.w:
         raise Exception('Attempt to save read-only "' + repr(self._filenam) + '"')
      if filename == self._filename:
         stderr.write('Warning: save destination was already "' + self._filename + '".\n')
      else:
         self._uptodate = filename == devnull
         self._filename = filename

   def find( self, key, ref = False):
      for d, k in self.nodes(lookfor = { key }):
         return d if ref else d[k]

   @staticmethod
   def _nodes( node , lookfor ):
      for k, v in node.items():
         if not lookfor or k in lookfor:
            if _numify(v) is None:
               yield node, k
            else:
               yield node, '$' + k
         if isinstance(v, dict):
            for t in naev_xml._nodes(v, lookfor):
               yield t

   def nodes( self, lookfor= None ):
      return self._nodes(self, lookfor)

   def touch( self ):
      self._uptodate = False

   def changed( self ):
      return not self._uptodate

   def __del__( self ):
      if self.w and not self._uptodate and self._filename != devnull:
         stderr.write('Warning: unsaved file "' + self._filename + '" at exit.\n')

if __name__ == '__main__':
   from sys import argv

   if inplace := '-i' in argv[1:]:
      argv.remove('-i')

   if '-h' in argv[1:]:
      stderr.write(
         'usage: ' + argv[0].split('/')[-1] + '  -i  [ file.xml.. ]\n'
         '  Reads a xml file, formats it and outputs the result.\n'
         '  If -i is set, does it in place.\n'
      )
      exit(0)
   for i in argv[1:]:
      n = naev_xml(i)
      if inplace:
         n.touch()
      else:
         n.save_as('-')
      n.save()
