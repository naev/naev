#!/usr/bin/python

keep_in_xml=set(['priority','rarity'])

from sys import argv,stderr,stdin,stdout,exit

from outfit import outfit,nam2fil,MOBILITY_PARAMS,text2val,roundit,ET


def fmt(f):
   return str(roundit(f))

def sfmt(f):
   res=fmt(f)
   if res=="0":
      return "_"
   elif res[0]!="-":
      return "+"+res
   else:
      return res;

def _process_group(r,field):
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
            a,b=text2val(e.text)
         except:
            continue

         if t == 'price':
            e.text=fmt(round((a+b)/2,-2))
         elif t=='priority' and a==b:
            continue
         else:
            if a==b:
               e.text=fmt(a)
            else:
               needs_lua=True
            acc.append((t,(a,b)))
            torem.append((r,e))

   return needs_lua,acc,torem

def _mklua(L):
   mods=''
   ind=3*' '

   output='\nrequire("outfits.lib.multicore").init{\n'

   for (nam,(main,sec)) in L:
      if nam not in keep_in_xml:
         output+=ind+'{ "'+nam+'",'+' '
         output+=fmt(main)+','+' '
         output+=fmt(sec)+'},'+'\n'
         if nam in MOBILITY_PARAMS:
            mods+=ind+'{ "'+nam+'_mod", 0, -50},\n'

   return output+mods+'}\n'

def toxmllua(o):
   T,R=o.T,o.r

   f1,acc1,tr1=_process_group(R,'./general')
   f2,acc2,tr2=_process_group(R,'./specific')

   if f1 or f2:
      for (r,e) in tr1+tr2:
         r.remove(e)

      for e in R.findall('./specific'):
         el=ET.Element("lua_inline")
         el.text=_mklua(acc1+acc2)
         e.append(el)
         break

if __name__ == '__main__':
   import argparse

   def main():
      o=outfit(stdin)
      if o is None:
         return 1
      else:
         name=o.name().rsplit(' (deprecated)',1)
         if len(name)==2 and name[1].strip()=='':
            o.set_name(name[0])
         nam=nam2fil(o.name())

         toxmllua(o)
         print >>stderr,nam
         o.write(stdout)
         return 0

   parser = argparse.ArgumentParser(
   description="""Takes an extended outfit as input on <stdin>, and produce a xml (potentially with inlined lua) on <stdout>.
         The name the output should have is written on <stderr>.
         If the input is invalid, nothing is written on stdout and stderr and non-zero is returned."""
   )
   parser.parse_args()
   exit(main())
