#!/usr/bin/env python3

import xml.etree.ElementTree as ET

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

   def parent( self ):
      return self._parent, self._key

   def __contains__ ( self, key ):
      key = key[1:] if key[:1] == '$' else key
      return key in self.att if key[:1] == '@' else dict.__contains__(self, key)

   def __getitem( self, key ):
      return self.attr[key] if key[:1] == '@' else dict.__getitem__(self, key)

   def __getitem__( self, key ):
      if isinstance(key, str) and key[0] == '$':
         return _numify(self.__getitem(key[1:]))
      else:
         return self.__getitem(key)

   def __setitem( self, key, val ):
      if key[:1] == '@':
         self.attr[key] = val
      else:
         dict.__setitem__(self, key, val)

   def __setitem__( self, key, val ):
      if isinstance(key, str) and key[0] == '$':
         val = _numify(val)
         if val is None:
            raise ValueError(str(val) + ' is not a number.')
         key = key[1:]
      elif isinstance(val, dict) and not isinstance(val, xml_node):
         val = xml_node(val, self, key)
      dict.__setitem__(self, key, val)
      self._change()

   def __delitem__( self, key ):
      if key[:1] == '@':
         del self.attr[key]
      else:
         dict.__delitem__(self, key, val)

   def __repr__( self ):
      return dict.__repr__(self.attr | self)

   __str__ = __repr__

class _trusted_node( xml_node ):
   def __init__ ( self, attr, parent= None, key= None ):
      self.attr = {('@'+k): v for k, v in attr.items()}
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

def parse( fn ):
   T = ET.parse(a).getroot()
   d = _trusted_node({})
   dict.__setitem__(d, T.tag, _parse(T, d, T.tag))
   return d

def _unparse_elt( v, k, indent):
   out = ''
   out += indent*' ' + '<' + k
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

from sys import argv

for a in argv[1:]:
   d = parse(a)
   with open(a, 'w') as fp:
      fp.write(unparse(d))

