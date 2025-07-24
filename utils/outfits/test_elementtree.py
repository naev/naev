#!/usr/bin/env python3

import xml.etree.ElementTree as ET
from sys import argv

def fmt_kv( kv ):
   (key, value) = kv
   return key + '="' + str(value) + '"'

def writeit( T, dst ):
   def output_r( e, fp, ind = 0 ):
      li = [e.tag] + [fmt_kv(x) for x in e.attrib.items()]

      try:
         iter(e).next()
         flag = True
      except:
         flag = False

      if e.text is None and not flag:
         fp.write(' '*ind+'<'+' '.join(li)+' />\n')
      else:
         fp.write(' '*ind+'<'+' '.join(li)+'>'+e.text.rstrip())
         fst = True
         for s in e:
            if fst:
               fp.write('\n')
               fst = False
            output_r(s, fp, ind+1)
         if not fst:
            fp.write(' '*ind)
         fp.write('</'+e.tag+'>\n')

   dest = open(dst, 'w')
   output_r(T.getroot(), dest)
   dest.close()

for a in argv[1:]:
   T = ET.parse(a)
   writeit(T, a)
