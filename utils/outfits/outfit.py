#!/usr/bin/env python

from sys import stdin,stdout,stderr

import xml.etree.ElementTree as ET

MOBILITY_PARAMS={'speed','turn','accel','thrust'}

def nam2fil(s):
   for c in [('Red Star','rs'),(' ','_'),('-',''),("'",''),('&','')]:
      s=s.replace(*c)
   return s.lower()

def text2val(s):
   try:
      inp=s.split('/',1)
      inp=[float(x) for x in inp]
      return (inp[0],inp[-1])
   except:
      return None

def roundit(f):
   return int(f) if f==round(f) else f

def fmtval(v):
   return str(roundit(v))

def andamp(s):
   return '' if s is None else s.replace("&","&amp;")

def fmt_kv(kv):
   (key,value)=kv
   return key+'="'+str(andamp(value))+'"'

def prisec(tag,r1,r2,eml1,eml2):
   a=r1[0] if r1 is not None else 0

   if r2 is not None:
      if tag in MOBILITY_PARAMS:
         a=(a*eml1+r2[1]*eml2)/(eml1+eml2)
      else:
         a+=r2[1]

   return roundit(a)

def stackvals(tag,text1,text2,eml1,eml2):
   return str(roundit(prisec(tag,text2val(text1),text2val(text2),eml1,eml2)))

def r_prisec(tag,v1,v2,eml1,eml2):
   if tag in MOBILITY_PARAMS:
      return v1,round((v2*(eml1+eml2) - eml1*v1)/float(eml2))
   else:
      return v1,v2-v1

def unstackvals(tag,text1,text2,eml1,eml2):
   o1,o2=r_prisec(tag,float(text1),0 if text2=='' else float(text2),eml1,eml2)
   if o2==o1:
      return fmtval(o1)
   else:
      return fmtval(o1)+'/'+fmtval(o2)

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
   def size(self,doubled=False):
      try:
         res=self.to_dict()['size']
         for i,k in enumerate(['small','medium','large']):
            if res==k:
               return 2*i+(2 if doubled else 1)
      except:
         pass

   def autostack(self,doubled=False):
      if doubled:
         try:
            res=self.to_dict()['engine_limit']
            if type(res)==type(()):
               (eml1,eml2)=res
            else:
               eml1=eml2=res
         except:
            eml1,eml2=None,None

      for e in self:
         res=text2val(e.text)
         if res is not None:
            if doubled:
               e.text=str(prisec(e.tag,res,res,eml1,eml2))
            else:
               e.text=str(prisec(e.tag,res,None,1,1))

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
         dest=open(dst,"w")
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

if __name__=="__main__":
   from sys import argv
   if len(argv)>1:
      stderr.write("Usage: "+argv[0].split('/')[-1]+'\n')
      stderr.write("  Reads a xml/mvx in input, outputs its input taken alone.\n")
   else:
      O=outfit(stdin)
      O.autostack()
      O.write()
