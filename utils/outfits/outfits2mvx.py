#!/usr/bin/python

from os import path
from sys import argv,stderr,exit,stdout
import xml.etree.ElementTree as ET
from outfit import roundit,stackvals,unstackvals

equals={'typename','slot','size'}
take_first={'description','outfit','gfx_store','priority','shortname'}
base_acc={'price','mass','rarity'}


if __name__ == '__main__':
   def merge_group(r1,r2,field,func,dummy=False):
      L1={e.tag:e for e in r1.findall(field)}
      L2=dict() if r2 is None else {e.tag:e for e in r2.findall(field)}

      if r2 is None and dummy:
         for t,e in L1.items():
            if t in base_acc:
               L2[t]=ET.Element(t)
               L2[t].text=str(roundit(float(L1[t].text)*2))
            else:
               L2[t]=e

      for t,e in L1.items():
         f=L2[t] if L2.has_key(t) else False

         if t in equals:
            if f is not False:
               if e.attrib!=f.attrib:
                  print >>stderr,t,":",e.attrib,"!=",f.attrib
                  exit(1)
               if e.text!=f.text:
                  print >>stderr,t,":",e.text,"!=",f.text
                  exit(1)
               e.set('prop_extra',e.attrib['prop']+'_secondary')
         elif t in take_first:
            pass
         else:
            rt=f.text if f is not False else ''
            try:
               e.text=func(e.tag,e.text,rt)
            except:
               print >>stderr,e,"is unmergeable, left as is."
      for t,f in L2.items():
         if not L1.has_key(t):
            try:
               f.text=func(e.tag,'',f.text)
               r1.append(f)
            except:
               print >>stderr,f,"is unmergeable, left as is."

   def main(args,func,stkmod):
      acc=[]
      T,R=[None,None],[None,None]
      m=[None,None]

      if len(args) not in [1,2]:
         print >>stderr,"One or two args expected"
         return 1

      dummy=not stkmod and len(args)==1

      for i in range(len(args)):
         print >>stderr,'<'+path.basename(args[i])+'>',
         T[i]=ET.parse(args[i])
         R[i]=T[i].getroot()
         for e in R[i].findall("./general/mass"):
            m[i]=float(e.text.split('(')[0])
            break
      print >>stderr

      if len(args)==2 and m[0]>m[1]:
         print >>stderr,"The first one has more mass, swapping",', '.join(args)
         args[0],args[1]=args[1],args[0]
         T,R=[T[1],T[0]],[R[1],R[0]]

      merge_group(R[0],R[1],'./general/',func,dummy)
      merge_group(R[0],R[1],'./specific/',func,dummy)

      T[0].write(stdout)

   if '-h' in argv[1:] or '--help' in argv[1:] or len(argv)<2:
      nam=path.basename(argv[0])
      print "usage (1):",nam,'<input1.xml> [<input2.xml>]'
      print "  Takes two standard outfits, and computes an extended outfit such that when in a main slot,"
      print "  it is eq to <input1.xml>, and stacking two of them gives <input2.xml>."
      print "  If <input2.xml> is not provided, the resulting outfit will be useless as a secondary."
      print "  The result is sent to stdout."
      print
      print "usage (2):",nam,'-s <main.xml> [<secondary.xml>]'
      print "  Takes one or two extended outfits, and computes the result of stacking them (or keeping it alone)."
      print "  The result is sent to stdout."
   else:
      ign=[f for f in argv[1:] if f not in ["-s","-l"] and not f.endswith(".xml")]
      if ign!=[]:
         print >>stderr,'Ignored: "'+'", "'.join(ign)+'"'

      merge_func=stackvals if '-s' in argv[1:] else unstackvals
      main([f for f in argv[1:] if f.endswith(".xml")],merge_func,'-s' in argv[1:])
      exit(0)
