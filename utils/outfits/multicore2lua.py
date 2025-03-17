#!/usr/bin/python

from os import path
from sys import argv,stderr,exit,stdin,stdout
import xml.etree.ElementTree as ET

mixed=False

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
            if mixed:
               acc.append((t,b-a))
               e.text=fmt(a)
            else:
               acc.append((t,(a,b)))
               torem.append(e)

   if not mixed:
      for e in torem:
         r.remove(e)

   return acc


"""
local fmt = require "format"

function descextra( _p, _o )
   local desc = ""

   local function add_desc( name, units, base, primary )
      if primary ~= 0 then
         desc = desc..fmt.f(_("{name}: {full) ({base}) {units}"), {
            name=name, units=units, base=base, full=base+primary,
         })
      else
         -- This could be just done in XML though...
         desc = desc..fmt.f(_("{name}: {full) {units}"), {
            name=name, units=units, base=base,
         })
      end
   end

   add_desc( _("Shield Strength"), naev.unit("energy"), SHIELD, PRI_SHIELD )
   return desc
end
"""

def mklua(luanam,L):
   print >>stderr,"<"+luanam+">"
   fp=file(luanam,"w")
   ind=3*' '

   print >>fp,"notactive = true"
   print >>fp
   print >>fp,"function init( _p, po )"

   if mixed:
      print >>fp,ind+"if po:slot().tags.secondary then"
      for (nam,sec) in L:
         print >>fp,ind*2+'po:set( "'+nam+'", '+fmt(sec)+' )'
      print >>fp,ind+"end"
      print >>fp,"end"
   else:
      for (nam,_) in L:
         print >>fp,ind+"local",nam
      print >>fp

      print >>fp,ind+"if not po:slot().tags.secondary then"

      for (nam,(main,sec)) in L:
         print >>fp,2*ind+nam+"="+fmt(main)

      print >>fp,ind+"else"
      for (nam,(main,sec)) in L:
         print >>fp,2*ind+nam+"="+fmt(sec)

      print >>fp,ind+"end"

      for (nam,_) in L:
         print >>fp,ind+'po:set( "'+nam+'", '+nam+' )'
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

