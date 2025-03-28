#!/usr/bin/env python

from sys import stdout,stderr

import xml.etree.ElementTree as ET

def nam2fil(s):
   for c in [('Red Star','rs'),(' ','_'),('-',''),("'",''),('&','')]:
      s=s.replace(*c)
   return s.lower()

class _outfit():
   def __init__(self,fil):
      if type(fil)==type(""):
         fp=open(fil,"rt")
         self.T=ET.parse(fp)
         fp.close()
      else:
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
      def _subs(r):
         for e in r:
            yield e
            for s in _subs(e):
               yield s

      for e in _subs(self.r):
         yield e

   def write(self,dst=stdout):
      def output_r(e,fp,ind=0):
         andamp=lambda s:s.replace("&","&amp;") if s is not None else ''
         def _fmt_a(kv):
            (key,value)=kv
            return key+'="'+str(andamp(value))+'"'

         li=[e.tag]+[_fmt_a(x) for x in e.attrib.items()]

         try:
            iter(e).next()
            flag=True
         except:
            flag=False

         if e.text is None and not flag:
            fp.write(' '*ind+'<'+' '.join(li)+' />\n')
         else:
            fp.write(' '*ind+'<'+' '.join(li)+'>'+andamp(e.text).rstrip())
            fst=True
            for s in e:
               if fst:
                  fp.write('\n')
                  fst=False
               output_r(s,fp,ind+1)
            if not fst:
               fp.write(' '*ind)
            fp.write('</'+e.tag+'>\n')

      closeit=False
      if dst=="-":
         dest=stdout
      elif type(dst)==type("foo"):
         dest=open(dst,"wt")
         closeit=True
      else:
         dest=dst

      output_r(self.r,dest)

      if closeit:
         dest.close()

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
   return _outfit(fil) if type(fil)!=type("foo") or fil.endswith(".xml") or fil.endswith('.mvx') else None

if __name__=="__main__":
   from sys import argv

   if len(argv)>1:
      O=outfit(argv[1])
      O.write()
