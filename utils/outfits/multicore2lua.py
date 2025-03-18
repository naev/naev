#!/usr/bin/python

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

         if a==b:
            e.text=fmt(a)
         elif t == 'price':
            e.text=fmt(round((a+b)/2,-2))
         else:
            acc.append((t,(a,b)))
            torem.append(e)

   for e in torem:
      r.remove(e)

   return acc

names={
   "priority":"Priority",
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
   "priority":"",
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

   print >>fp,"notactive = true\n"
   print >>fp,'local fmt = require "format"\n'
   print >>fp,'local nomain=false'
   print >>fp,'local nosec=false'

   print >>fp,"""
function descextra( _p, po )
   local desc = ""

   print(fmt.f("desc {po}",{po=po}))
   local function _vu( val, unit)
      if val=='_' then
         return val
      else
         return val.." "..unit
      end
   end

   local function vu( val, unit, grey, def)
      local res=_vu(val,unit)
      if grey then
         return "#b"..res..def
      else
         return res
      end
   end

   local function add_desc( name, units, base, secondary, def)
      desc = desc..fmt.f(_("\\n{name}: {bas} / {sec}"), {
         name=name, units=units, bas=vu(base,units,nomain,def), sec=vu(secondary,units,nosec,def)
      })
   end
"""
   for (nam,(main,sec)) in L:
      if units.has_key(nam):
         if nam=="mass":
            defa='"#r"'
            print >>fp,ind+'desc=desc.."#r"'
         else:
            defa='"#g"'

         if units[nam]!='':
            print >>fp,ind+'add_desc( _("'+names[nam]+'"), naev.unit("'+units[nam]+'"),', '"'+sfmt(main)+'","',sfmt(sec)+'",',defa,')'
         else:
            print >>fp,ind+'add_desc( _("'+names[nam]+'"), "",', '"'+sfmt(main)+'","',sfmt(sec)+'"',',',defa,')'
         if nam=="mass":
            print >>fp,ind+'desc=desc.."#g"'
      else:
         print >>stderr,"unknown unit of",repr(nam)

   print >>fp,"""
   return desc
end
"""
   print >>fp,"function init(_p, po )"
   print >>fp,ind+'print(fmt.f("init {po}",{po=po}))'

   print >>fp,ind+"if po:slot().tags and po:slot().tags.core then"
   for (nam,_) in L:
      print >>fp,2*ind+"local",nam

   print >>fp,2*ind+"if not po:slot().tags.secondary then"
   print >>fp,3*ind+'nosec=true'
   print >>fp,3*ind+'nomain=false'
   for (nam,(main,sec)) in L:
      print >>fp,3*ind+nam+"="+fmt(main)

   print >>fp,2*ind+'else'
   print >>fp,3*ind+'nosec=false'
   print >>fp,3*ind+'nomain=true'
   for (nam,(main,sec)) in L:
      print >>fp,3*ind+nam+"="+fmt(sec)
   print >>fp,2*ind+"end"

   for (nam,_) in L:
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

   acc+=process_group(R,'./general')
   acc+=process_group(R,'./specific')

   if acc!=[]:
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

