#!/usr/bin/python

from sys import argv,stderr,exit,stdin,stdout
import xml.etree.ElementTree as ET


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
   for e in r.findall(field):
      t=e.tag
      try:
         a,b=read_com(e.text)
         if a==b:
            e.text=fmt(a)
         else:
            acc.append((t,(a,b)))
            e.remove()
      except:
         pass
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

   T=ET.parse(stdin)
   R=T.getroot()

   acc+=process_group(R,'./general/')
   acc+=process_group(R,'./specific/')

   if acc!=[]:
      for e in R.findall('./specific'):
         el=ET.Element("lua")
         el.text=arg+".lua"
         e.append(el)
         break
      mklua(arg+".lua",acc)
   else:
      print >>stderr,"No composite field found, left as is."

   T.write(arg+".xml")
   print >>stderr,"<"+arg+".xml"+">"


if __name__ == '__main__':
   if '-h' in argv[1:] or '--help' in argv[1:] or len(argv)<2:
      nam=argv[0].split('/')[-1]
      print "usage:",nam,'<output_name>'
      print "  Takes an extended outfit as input on stdin, and computes <output_name>.{xml,lua}."
   else:
      ign=argv[2:]
      if ign!=[]:
         print >>stderr,'Ignored: "'+'", "'.join(ign)+'"'

      main(argv[1])
      exit(0)

