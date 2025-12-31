#!/usr/bin/env python3

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

class _xml_list( list ):
   def __init__( self, parent, key, L=[] ):
      self._parent = parent
      self._key = key
      list.__init__(self, [_xml_ify(i, parent, key) for i in L])

   def _change( self ):
      self._parent._change()

   def __setitem__( self, key, value ):
      if isinstance(key, slice):
         list.__setitem__(self, key, _xml_list(self._parent, self._key, value))
      else:
         list.__setitem__(self, key, _xml_ify(value, self._parent, self._key))
      self._change()

   def insert( self, index, elt ):
      list.insert(self, index, _xml_ify(elt, self._parent, self._key))
      self._change()

   def append( self, elt ):
      list.append(self, _xml_ify(elt, self._parent, self._key))
      self._change()

   def extend( self, other ):
      list.extend(self, _xml_list(self._parent, self._key, other))
      self._change()

   __iadd__ = extend

   def clear( self ):
      self._change()
      list.clear(self)

   def pop( self, index= -1 ):
      self._change()
      return list.pop(self, index)

   def remove( self, val ):
      self._change()
      list.remove(self, val)

   def reverse( self ):
      self._change()
      list.reverse(self)

   def sort( self, **args ):
      self._change()
      list.sort(self, **args)

   def __delitem__( self, item ):
      self._change()
      list.__delitem__(self, item)

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
      if val is None:
         if key in self:
            del self[key]
         return
      elif isinstance(key, str) and key[0] == '$':
         val = _numify(val)
         if val is None:
            raise ValueError(str(val) + ' is not a number.')
         key = key[1:]
      elif isinstance(val, list):
         val = _xml_list(self, key, val)
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
      self._change()

   def __repr__( self ):
      return dict.__repr__(self.attr | self)

   __str__ = __repr__

   def clear(self):
      dict.clear(self)
      self.attr.clear()
      self._change()

   # Do these make any sense in this context ?
   popitem = None
   pop = None

   # update does not rely on setitem, contrary to the doc
   # This is the implementation corresponding to the doc.
   def update( self, *E, **F ):
      if E:
         E,*_ = E
         if hasattr(E, 'keys'):
            for k in E.keys():
               self[k] = E[k]
         else:
            for k, v in E:
               self[k] = v
      for k in F:
         self[k] = F[k]

   def __ior__( self, other ):
      self.update(other)
      return self

   def keys():
      return self.attr.keys() + list.keys(self)

   def setdefault( self, key, default= None ):
      self._change()
      return dict.setdefault(self, key, default)


def _xml_ify( elt, par, k ):
   return xml_node(elt, par, k) if isinstance(elt, dict) else elt

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
         crt = dict.__getitem__(d, e.tag)
         if not isinstance(crt, list):
            l = _xml_list(d, e.tag)
            list.append(l, crt)
            dict.__setitem__(d, e.tag, l)
         list.append(dict.__getitem__(d, e.tag), _parse(e, d, e.tag))
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
      self.short = None
      if fnam is None:
         r = False
         fnam = devnull
      self._uptodate = r
      self.w, self.r = w, r
      if type(fnam) != type('') or ('xml' not in fnam.split('.') and fnam != devnull):
         raise Exception('Invalid xml filename ' + repr(fnam))
      self._filename = fnam

      self.short = False
      _trusted_node.__init__(self, {}, None, None)
      if r and self._filename != devnull:
         try:
            P = ET.parse(self._filename)
         except FileNotFoundError as e:
            raise e
         # Can also raise:
         #  - xml.etree.ElementTree.ParseError: ...
         except Exception as e:
            raise Exception('Invalid xml file ' + repr(self._filename)) from e

         T = P.getroot()
         try:
            dict.__setitem__(self, T.tag, _parse(T, self, T.tag))
         except:
            raise Exception('Invalid xml file ' + repr(self._filename))

   def save( self, if_needed= False ):
      if not self.w:
         raise Exception('Attempt to save read-only ' + repr(self._filenam))
      if self._uptodate:
         if if_needed:
            return False
         else:
            stderr.write('Warning: saving unchanged file ' + repr(self._filename) + '.\n')

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
         raise Exception('Attempt to save read-only ' + repr(self._filenam))
      if filename == self._filename:
         stderr.write('Warning: save destination was already ' + repr(self._filename) + '.\n')
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
            yield from naev_xml._nodes(v, lookfor)

   def nodes( self, lookfor= None ):
      return self._nodes(self, lookfor)

   @staticmethod
   def _to_dict( node ):
      for k, v in node.items():
         if isinstance(v, dict):
            yield from naev_xml._to_dict(v)
         elif (vv := _numify(v)) is not None:
            yield k, vv

   def to_dict( self ):
      return dict(self._to_dict(self))

   def name( self ):
      return next(iter(self.values()))['@name']

   def shortname( self ):
      if not self.short:
         if (res := self.find('shortname')) is None:
            res = self.name()
         if res.split(' ')[-1] == 'Engine':
            res = ' '.join(res.split(' ')[:-1])
         self.short = res
      return self.short

   def touch( self ):
      self._uptodate = False

   def changed( self ):
      return not self._uptodate

   def __del__( self ):
      if self.w and not self._uptodate and self._filename != devnull:
         stderr.write('Warning: unsaved file ' + repr(self._filename) + ' at exit.\n')

def xml_parser( constr, qualifier= '' ):
   from pathlib import Path
   from sys import argv
   from os import path
   from pathlib import Path

   args = argv[1:]
   if inplace := '-i' in args:
      args.remove('-i')

   if clone := '-c' in args:
      args.remove('-c')
      src, dst = (args + [None, None])[:2]
      args = args[2:]

   if args == [] or '-h' in args or (clone and inplace):
      stderr.write(
         'usage: ' + argv[0].split('/')[-1] + ' [ -i | (-c <src_path> <dst_path>)] [ file.xml.. ]\n'
         '  Reads a ' + qualifier + 'xml file, formats it and outputs the result.\n'
         '  If -i (in-place) is set, does it in place.\n'
         '  If -c (clone) is set, for each <file.xml>, produces an uncommented version\n'
         '  that is written to a file that has the same position in <dst_path> that\n'
         '  <file.xml> has in <src_path>.\n'
      )
      exit(0)
   for arg in args:
      x = constr(arg)
      if inplace:
         x.touch()
      elif clone:
         if (rel := path.relpath(path.realpath(arg), src))[:2] == '..':
            stderr.write('\033[31merror:\033[0m file ' + repr(arg) + ' not in ' + repr(src) +'\n')
            exit(1)
         oarg = path.join(dst, rel)
         Path(path.split(oarg)[0]).mkdir(parents= True, exist_ok= True)
         x.save_as(oarg)
      else:
         x.save_as('-')
      x.save()

if __name__ == '__main__':
   xml_parser(naev_xml)
