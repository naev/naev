#!/usr/bin/env python

from sys import stdin,stdout,stderr

import xml.etree.ElementTree as ET

MOBILITY_PARAMS={'speed','turn','accel','thrust'}

def nam2fil(s):
   for c in [('Red Star','rs'),(' ','_'),('-',''),("'",''),('&','')]:
      s=s.replace(*c)
   return s.lower()

def text2val(s):
   inp=s.split('/',1)
   try:
      inp=[float(x) for x in inp]
      return (inp[0],inp[-1])
   except:
      return None

def roundit(f):
   return int(f) if f==round(f) else f

def andamp(s):
   return '' if s is None else s.replace("&","&amp;")

def fmt_kv(kv):
   (key,value)=kv
   return key+'="'+str(andamp(value))+'"'

def prisec(tag,r1,r2):
   if r1 is not None:
      a=r1[0]
   else:
      a=0

   if r2 is not None:
      a+=r2[1]
      if tag in MOBILITY_PARAMS:
         a/=2.0

   return roundit(a)

def rprisec(tag,v1,v2):
   if tag in MOBILITY_PARAMS:
      v2*=2
   return v1,v2-v1

def stackvals(tag,text1,text2):
   return str(prisec(tag,text2val(text1),text2val(text2)))

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

   def set_name(self,name):
      self.r.attrib['name']=name

   def shortname(self):
      try:
         res=self.to_dict()['shortname']
      except:
         res=self.name()
      return res

   def autostack(self,doubled=False):
      for e in self:
         res=text2val(e.text)
         if res is not None:
            e.text=str(prisec(e.tag,res,res if doubled else None))

   def __iter__(self):
      def _subs(r):
         for e in r:
            yield e
            for s in _subs(e):
               yield s

      return iter(_subs(self.r))

   def write(self,dst=stdout):
      def output_r(e,fp,ind=0):
         li=[e.tag]+[fmt_kv(x) for x in e.attrib.items()]

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
      elif type(dst)==type(""):
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
   if type(fil)!=type("") or fil.endswith(".xml") or fil.endswith('.mvx'):
      o=_outfit(fil)
      if o.r.tag=='outfit':
         return o
   return None

if __name__=="__main__":
   from sys import argv
   if len(argv)>1:
      stderr.write("Usage: "+argv[0].split('/')[-1]+'\n')
      stderr.write("  Reads a xml/mvx in input, outputs its input taken alone.\n")
   else:
      O=outfit(stdin)
      O.autostack()
      O.write()
