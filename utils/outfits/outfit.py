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
   fp.write(' '*ind+'<'+' '.join([e.tag]+[fmt_a(x) for x in e.attrib.items()])+'>'+e.text.strip())
   fst=True
   for s in e:
      if fst:
         fp.write('\n')
         fst=False
      output_r(s,fp,ind+1)
   if not fst:
      fp.write(' '*ind)
   fp.write('</'+e.tag+'>\n')

class _outfit():
   def __init__(self,fil):
      self.T=ET.parse(fil)
      self.r=self.T.getroot()
      self.fil=fil

   def name(self):
      return self.r.attrib['name']

   def shortname(self):
      try:
         res=self.to_dict()['shortname']
      except:
         res=self.name()
      return res

   def __iter__(self):
      for e in subs(self.r):
         yield e

   def write(self,fp=stdout):
      output_r(self.r,fp)

   def to_dict(self):
      d=dict()
      for k in self:
         if not k.tag in d:
            d[k.tag]=[]
         what=k.text
         if len(what.split('/'))<=2:
            try:
               what=tuple(map(float,what.split('/')))
               if len(what)==1:
                  what=what[0]
            except:
               pass
         d[k.tag].append(what)
      for k in d:
         if len(d[k])==1:
            d[k]=d[k][0]
      return d

def outfit(fil):
   return _outfit(fil) if fil.endswith(".xml") or fil.endswith('.mvx') else None

if __name__=="__main__":
   from sys import argv

   if len(argv)>1:
      O=outfit(argv[1])
      O.write()
