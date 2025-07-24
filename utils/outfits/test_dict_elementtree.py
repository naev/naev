#!/usr/bin/env python3

import xml.etree.ElementTree as ET
from xmltodict import unparse
from sys import argv


def _parse(node):
   d = { ('@' + k): v for k, v in node.attrib.items() }

   for e in node:
      if e.tag in d:
         if not isinstance(d[e.tag], list):
            d[e.tag] = [d[e.tag]]
         d[e.tag] += [_parse(e)]
      else:
         d[e.tag] = _parse(e)

   t = node.text
   t = t and t.strip() or None
   if d == {}:
      return t
   if t:
      d['#text'] = t
   return d

def parse(fn):
   T = ET.parse(a).getroot()
   return {T.tag:_parse(T)}

def fmt_kv( kv ):
   (key, value) = kv
   return key + '="' + str(value) + '"'

for a in argv[1:]:
   with open(a, 'r') as fp:
      d = parse(a)
   with open(a, 'w') as fp:
      fp.write(unparse(d, pretty= True, indent= ' ', full_document= False) + '\n')
