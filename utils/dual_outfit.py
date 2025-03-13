#!/usr/bin/python

from sys import argv,stderr,exit,stdout
import xml.etree.ElementTree as ET


equals={'typename','slot','size'}
take_first={'description','outfit','gfx_store','priority'}

def merge_elements(s1,s2,f):
   n1=float(s1)
   n2=float(s2)
   return f(n1,n2)

def merge_group(r1,r2,field,func):
   L1={e.tag:e for e in r1.findall(field)}
   L2={e.tag:e for e in r2.findall(field)}

   for t,e in L1.items():
      f=L2[t] if L2.has_key(t) else ''
      if t in equals:
         if e.attrib!=f.attrib:
            print >>stderr,t,":",e.attrib,"!=",f.attrib
            exit(1)
         if e.text!=f.text:
            print >>stderr,t,":",e.text,"!=",f.text
            exit(1)
      elif t in take_first:
         pass
      else:
         try:
            e.text=merge_elements(e.text,f.text,func)
         except:
            print >>stderr,e,"is unmergeable, left as is."
   for t,f in L2.items():
      if not L1.has_key(t):
         print >>stderr,"forgot:",t

def main(args,func=lambda f1,f2:str(f1)+','+str(f2)):
   names=['']*len(args)
   tags=set([])
   acc=[]
   T=[None,None]
   R=[None,None]
   m=[None,None]

   if len(args)!=2:
      print >>stderr,"Two args expected"
      return

   for i in range(2):
      print >>stderr,'<'+args[i].rsplit('/',1)[-1]+'>',
      T[i]=ET.parse(args[i])
      R[i]=T[i].getroot()
      for e in R[i].findall("./general/mass"):
         m[i]=int(e.text)
         break;
   print >>stderr

   if m[0]>m[1]:
      print >>stderr,"The first one has more mass, swapping",', '.join(args)
      args[0],args[1]=args[1],args[0]
      T=[T[1],T[0]]
      R=[R[1],R[0]]

   merge_group(R[0],R[1],'./general/',func)
   merge_group(R[0],R[1],'./specific/',func)

   T[0].write(stdout)
   print >>stderr,"Output sent to <stdout>"

def fmt(f):
   if f==round(f):
      return str(int(f))
   else:
      return str(f)

def f1(a1,a2):
   o1=a2-a1
   o2=2*a1-a2
   if o2==0:
      return fmt(o1)
   else:
      return fmt(o1)+'('+fmt(o2)+')'

def f2(a1,a2):
   o1=a1
   o2=a2/2
   return fmt(o1)+'('+fmt(o2)+')'

if __name__ == '__main__':
   if '-h' in argv[1:] or '--help' in argv[1:] or len(argv)<2:
      nam=argv[0].split('/')[-1]
      print "usage (1):",nam,' [-l] <input1.xml> <input2.xml>'
      print "  Takes two standard outfits, and computes an extended outfit such that when in a main slot,"
      print "  it is eq to <input1.xml>, and stacking two of them gives <input2.xml>."
      print "  The result is sent to stdout."
      print "   -l use the Lone(twinned) variant"
      print
      print "usage (2):",nam,' -s <main.xml> [<secondary.xml>]'
      print "  Takes one or two extended outfits, and computes the result of stacking them (or keeping it alone)."
      print "  This is \033[1;31mNOT IMPLEMENTED\033[0m (yet)."
   else:
      ign=[f for f in argv[1:] if f not in ["-s","-l"] and not f.endswith(".xml")]
      if ign!=[]:
         print >>stderr,'Ignored: "'+'", "'.join(ign)+'"'

      if '-s' in argv[1:]:
         print >>stderr,"not implemented!"
         exit(1)
      else:
         if '-l' in argv[1:]:
            merge_func=f2
         else:
            merge_func=f1
         main([f for f in argv[1:] if f.endswith(".xml")],merge_func)
         exit(0)

