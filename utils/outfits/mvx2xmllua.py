#!/usr/bin/python

dont_display=set(['priority','rarity'])
from outfit import outfit

#TODO: use argparse

from os import path
from sys import argv,stderr,exit,stdin,stdout
import xml.etree.ElementTree as ET

from outfit import nam2fil

def get_path(s):
   s=path.dirname(s)

   if s=='':
      return ''
   else:
      return s+path.sep

def read_com(s):
   if s=='':
      return 0,0
   elif '/' in s:
      n,m=tuple(s.split('/',1))
      return float(n),float(m)
   else:
      return float(s),float(s)

def fmt(f):
   if f==round(f):
      return str(int(f))
   else:
      return str(f)

def sfmt(f):
   res=fmt(f)
   if res=="0":
      return "_"
   elif res[0]!="-":
      return "+"+res
   else:
      return res;

def process_group(r,field):
   needs_lua=False
   acc=[]
   r=r.find(field)
   torem=[]
   for e in r.iter():
      t=e.tag
      if t == 'slot':
         e.set('prop_extra',e.attrib['prop']+'_secondary')
      else:
         try:
            a,b=read_com(e.text)
         except:
            continue

         if t == 'price':
            e.text=fmt(round((a+b)/2,-2))
         elif a==b:
            e.text=fmt(a)
            acc.append((t,(a,a)))
         else:
            needs_lua=True
            acc.append((t,(a,b)))
            torem.append(e)

   for e in torem:
      r.remove(e)

   return needs_lua,acc


def mklua(L):
   output='\n'
   ind=3*' '

   output+='require("outfits.lib.multicore").init{\n'

   for (nam,(main,sec)) in L:
      if nam not in dont_display:
         output+=ind+'{ "'+nam+'",'+' '
         output+=fmt(main)+','+' '
         output+=fmt(sec)+'},'+'\n'

   return output+'}\n'

def main():
   acc=[]

   o=outfit(stdin)
   T=o.T
   R=o.r

   if R.tag!='outfit':
      print >>stderr,"not an outfit :",R.tag
      return 1

   nam=R.attrib['name'].rsplit(' (deprecated)',1)
   if len(nam)==1 or nam[1].strip()=='':
      R.attrib['name']=nam[0]

   nam=nam2fil(R.attrib['name'])

   f1,acc1=process_group(R,'./general')
   f2,acc2=process_group(R,'./specific')

   if f1 or f2:
      acc=acc1+acc2

      for e in R.findall('./specific'):
         el=ET.Element("lua_inline")
         el.text=mklua(acc)
         e.append(el)
         break
   else:
      pass
      #print >>stderr,"No composite field found, left as is."

   print >>stderr,nam
   o.write(stdout)
   
if __name__ == '__main__':
   if '-h' in argv[1:] or '--help' in argv[1:] or len(argv)!=1:
      nam=path.basename(argv[0])
      print >>stderr, "usage:",nam
      print >>stderr, "  Takes an extended outfit as input on <stdin>, and produce a xml (potentially with inlined lua) on <stdout> and the name the output should have on <stderr>."
   else:
      main()
      exit(0)
