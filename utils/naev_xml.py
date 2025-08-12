# python3

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

import xml.etree.ElementTree as ET
from sys import stderr
from os import devnull

def _numify( s ):
   try:
      f = float(s)
      return int(f) if round(f) == f else f
   except:
      pass

class xml_node( dict ):
   def __init__ ( self, mapping, parent= None, key= None ):
      mknode = lambda v: xml_node(v) if isinstance(v, dict) else v
      self.attr = {k: v for k, v in mapping.items() if k[:1]=='@'}
      dict.__init__(self, {k: v for k, v in mapping.items() if k[:1]!='@'})
      self._parent = parent
      self._key = key

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
      elif isinstance(val, dict) and not isinstance(val, xml_node):
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
   if isinstance(v, dict):
      for ka, va in v.attr.items():
         out += ' ' + ka[1:] + '="' + va.replace('&', '&amp;') + '"'
      if v == {}:
         out += '/>\n'
      else:
         out += '>'
         if not dict.__contains__(v, '#text'):
            out += '\n'
            out += unparse(v, indent+1)
            out += indent*' '
         else:
            out += v['#text']
         out += '</' + k + '>\n'
   elif v:
      out += '>'
      out += str(v).replace('&', '&amp;')
      out += '</' + k + '>\n'
   else:
      out += '/>\n'
   return out

def unparse( d , indent= 0 ):
   out = ''
   for k, v in d.items():
      if isinstance(v, list):
         for ve in v:
            out += _unparse_elt(ve, k, indent)
      else:
         out += _unparse_elt(v, k, indent)

   return out

class naev_xml( xml_node ):
   def __init__( self, fnam= devnull, read_only= False ):
      self._uptodate = True
      if type(fnam) != type('') or (not fnam.endswith('.xml') and fnam != devnull):
         raise Exception('Invalid xml filename ' + repr(fnam))
      self.short = False
      _trusted_node.__init__(self, {}, None, None)
      if fnam != devnull:
         T = ET.parse(fnam).getroot()
         dict.__setitem__(self, T.tag, _parse(T, self, T.tag))

      self._filename = devnull if read_only else fnam

   def save( self ):
      if self._uptodate:
         stderr.write('Warning: saving unchanged file "' + self._filename + '".\n')

      with open(self._filename, 'w') as fp:
         fp.write(unparse(self))

      self._uptodate = True

   def save_as( self, filename ):
      self._uptodate = (self._uptodate and filename == self._filename) or filename == devnull
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
      if not self._uptodate and self._filename != devnull:
         stderr.write('Warning: unsaved file "' + self._filename + '" at exit.\n')
