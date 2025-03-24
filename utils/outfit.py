#!/usr/bin/python

from sys import stdout,stderr
import xml.etree.ElementTree as ET

def subs(r):
   for e in r:
      yield e
      for s in subs(e):
         yield s

def fmt_a(kv):
   (key,value)=kv
   return key+'="'+str(value)+'"'

def output_r(e,fp,ind=0):
   fp.write(' '*ind+'<'+' '.join([e.tag]+map(fmt_a,e.attrib.iteritems()))+'>'+e.text.strip())
   fst=True
   for s in e:
      if fst:
         fp.write('\n')
         fst=False
      output_r(s,fp,ind+1)
   if not fst:
      fp.write(' '*ind)
   fp.write('</'+e.tag+'>\n')

class outfit():
   def __init__(self,fil):
      self.T=ET.parse(fil)
      self.r=self.T.getroot()

   def name(self):
      return self.r.attrib['name']

   def __iter__(self):
      for e in subs(self.r):
         yield e

   def write(self,fp=stdout):
      output_r(self.r,fp)

if __name__=="__main__":
   from sys import argv

   if len(argv)>1:
      O=outfit(argv[1])
      O.write()

