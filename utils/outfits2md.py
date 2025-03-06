#!/usr/bin/python

from sys import argv
import xml.etree.ElementTree as ET


def main(args):
   names=['']*len(args)
   L=[dict() for a in args]
   done=set()
   acc=[]

   for i in range(len(args)):
      T=ET.parse(args[i]).getroot()
      for t in T.iter():
         try:
            n=float(t.text)
            L[i][t.tag]=t.text
            if t.tag not in done:
               done.add(t.tag)
               acc.append(t.tag)
         except:
            pass

      if 'name' in T.attrib:
         names[i]=T.attrib['name']

      for e in T.findall("./general/shortname"):
         names[i]=e.text
         break

   Res=[[k]+[l[k] if k in l else '' for l in L] for k in acc]
   names=['']+names
   length=map(len,names)

   for r in Res:
      length=[max(n,len(s)) for (n,s) in zip(length,r)]

   mklin=lambda L:'| '+' | '.join(L)+' |'
   fmt=lambda (s,n):(n-len(s))*' '+s

   print mklin(map(fmt,zip(names,length)))
   print mklin(['-'*n for n in length])
   for r in Res:
      print mklin(map(fmt,zip(r,length)))

if '-h' in argv[1:] or '--help' in argv[1:] or len(argv)<2:
   print "usage:",argv[0],'<outfitname1.xml> ...'
else:
   main([f for f in argv[1:] if f.endswith(".xml")])

