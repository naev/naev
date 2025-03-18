#!/usr/bin/python

dont_display=set(['priority'])

from os import path
from sys import argv,stderr,exit,stdin,stdout
import xml.etree.ElementTree as ET

def get_path(s):
   s=path.dirname(s)

   if s=='':
      return ''
   else:
      return s+path.sep

def nam2fil(s):
   return s.replace(' ','_').replace('-','').replace("'",'').lower()

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
         e.attrib['prop_extra']="systems_secondary"
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

names={
   "mass":"Ship Mass",
   "cpu_max":"CPU max",
   "energy":"Energy Capacity",
   "energy_regen":"Energy Regeneration",
   "shield":"Shield Capacity",
   "shield_regen":"Shield Regeneration",
   "ew_detect":"Detection",
   "cooldown_time":"Ship Cooldown Time"
}
units={
   "mass":"mass",
   "cpu_max":"",
   "energy":"energy",
   "energy_regen":"",
   "shield":"energy",
   "shield_regen":"",
   "ew_detect":"",
   "cooldown_time":""
}

def mklua(luanam,L):
   print >>stderr,"<"+luanam+">"
   fp=file(luanam,"w")
   ind=3*' '

   print >>fp,"""notactive = true
local nomain=false
local nosec=false
local add_desc=require "outfits.multicores.desc"

function descextra( _p, _po )
   local desc = ""
"""

   for (nam,(main,sec)) in L:
      if nam not in dont_display:
         if units.has_key(nam):
            if nam=="mass":
               defa='"#r"'
               print >>fp,ind+'desc=desc.."#r"'
            else:
               defa='"#g"'

            if units[nam]!='':
               print >>fp,ind+'desc=add_desc(desc, _("'+names[nam]+'"), naev.unit("'+units[nam]+'"),', '"'+sfmt(main)+'", "'+sfmt(sec)+'",',defa+', nomain, nosec)'
            else:
               print >>fp,ind+'desc=add_desc(desc, _("'+names[nam]+'"), "",', '"'+sfmt(main)+'", "'+sfmt(sec)+'"',',',defa+', nomain, nosec)'

            if nam=="mass":
               print >>fp,ind+'desc=desc.."#g"'
         else:
            print >>stderr,"unknown unit of",repr(nam)

   print >>fp,"""
   return desc
end
"""
   L2=[(nam,(a,b)) for (nam,(a,b)) in L if a!=b]
   print >>fp,"function init(_p, po )"
   print >>fp,ind+"if po:slot().tags and po:slot().tags.core then"
   for (nam,_) in L2:
      print >>fp,2*ind+"local",nam

   print >>fp,2*ind+"if not po:slot().tags.secondary then"
   print >>fp,3*ind+'nosec=true'
   print >>fp,3*ind+'nomain=false'
   for (nam,(main,sec)) in L2:
      print >>fp,3*ind+nam+"="+fmt(main)

   print >>fp,2*ind+'else'
   print >>fp,3*ind+'nosec=false'
   print >>fp,3*ind+'nomain=true'
   for (nam,(main,sec)) in L2:
      print >>fp,3*ind+nam+"="+fmt(sec)
   print >>fp,2*ind+"end"

   for (nam,_) in L2:
      print >>fp,2*ind+'po:set( "'+nam+'", '+nam+' )'

   print >>fp,ind+'else'
   print >>fp,2*ind+'nosec=false'
   print >>fp,2*ind+'nomain=false'
   print >>fp,ind+"end"

   print >>fp,"end"
   fp.close()

def main(arg):
   acc=[]
   path=get_path(arg)

   T=ET.parse(stdin)
   R=T.getroot()

   if R.tag!='outfit':
      print >>stderr,"not an outfit :",R.tag
      return 1

   nam=R.attrib['name'].rsplit(' (deprecated)',1)
   if len(nam)==1 or nam[1].strip()=='':
      R.attrib['name']=nam[0]

   nam=nam2fil(R.attrib['name'])
   print nam

   f1,acc1=process_group(R,'./general')
   f2,acc2=process_group(R,'./specific')

   if f1 or f2:
      acc=acc1+acc2

      for e in R.findall('./specific'):
         el=ET.Element("lua")
         el.text=(path+nam+".lua").split('dat/',1)[-1]
         e.append(el)
         break
      """
      el=ET.Element("lua")
      path=arg.split('dat/',1)[-1]
      el.text=path+".lua"
      R.append(el)
      """
      mklua(path+nam+".lua",acc)
   else:
      print >>stderr,"No composite field found, left as is."

   output=path+nam+".xml"
   T.write(output)
   print >>stderr,"<"+output+">"

if __name__ == '__main__':
   if '-h' in argv[1:] or '--help' in argv[1:] or len(argv)<2:
      nam=path.basename(argv[0])
      print >>stderr, "usage:",nam,'<output_path>'
      print >>stderr, "  Takes an extended outfit as input on stdin, and computes <output_name>.{xml,lua}."
   else:
      ign=argv[2:]
      if ign!=[]:
         print >>stderr,'Ignored: "'+'", "'.join(ign)+'"'

      main(argv[1])
      exit(0)

