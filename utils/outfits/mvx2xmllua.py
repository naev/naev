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
   is_engine=False
   acc=[]
   r=r.find(field)
   torem=[]
   for e in r.iter():
      t=e.tag
      try:
         a,b=text2val(e.text)
      except:
         if t == 'slot' and e.attrib['prop']=='engines':
            is_engine=True
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

   return needs_lua,acc,torem,is_engine

def _mklua(L,dual_eng=True):
   mods=''
   ind=3*' '

   if L==[]:
      return '\n'

   output='\nrequire("outfits.lib.multicore").init{\n'

   for (nam,(main,sec)) in L:
      if nam not in keep_in_xml:
         output+=ind+'{ "'+nam+'",'+' '
         output+=fmt(main)+','+' '
         output+=fmt(sec)+'},'+'\n'

   return output+mods+'}\n'

def toxmllua(o,update_lua,fake_dual):
   T,R=o.T,o.r

   f1,acc1,tr1,e1=_process_group(R,'./general')
   f2,acc2,tr2,e2=_process_group(R,'./specific')

   if (not f1) and (not f2) and (not update_lua):
      tr1=tr2=[]
      acc1=acc2=[]

   if f1 or f2 or update_lua or fake_dual or e1 or e2:
      for (r,e) in tr1+tr2:
         r.remove(e)

      for e in R.findall('./specific'):
         el=ET.Element("lua_inline")
         el.text=_mklua(acc1+acc2,not fake_dual and (e1 or e2))
         if update_lua:
            el.text+='require "outfits.lib.'+update_lua+'"\n'
         e.append(el)
         break

      if e1 or e2:
         e=ET.Element('tag')
         f=ET.Element('tag')
         e.append(f)
         R.append(e)


if __name__ == '__main__':
   import argparse

   def main(update_lua,fake_dual):
      o=outfit(stdin)
      if o is None:
         return 1
      else:
         name=o.name().rsplit(' (deprecated)',1)
         if len(name)==2 and name[1].strip()=='':
            o.set_name(name[0])
         nam=nam2fil(o.name())

         toxmllua(o,update_lua,fake_dual)
         print >>stderr,nam
         o.write(stdout)
         return 0

   parser = argparse.ArgumentParser(
   description="""Takes an extended outfit as input on <stdin>, and produce a xml (potentially with inlined lua) on <stdout>.
         The name the output should have is written on <stderr>.
         If the input is invalid, nothing is written on stdout and stderr and non-zero is returned."""
   )
   parser.add_argument('lua_module', nargs='?', help='The name of a lua module returning update function.')
   args=parser.parse_args()
   exit(main(args.lua_module,args.lua_module=='alone'))
