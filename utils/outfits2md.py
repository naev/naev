#!/usr/bin/python

from sys import argv
import xml.etree.ElementTree as ET


def main(args):
   names=['']*len(args)
   L=[dict() for a in args]
   rang=dict()
   acc=[]

   for i in range(len(args)):
      T=ET.parse(args[i]).getroot()
      for t in T.iter():
         try:
            n=float(t.text)
            L[i][t.tag]=t.text
            if not rang.has_key(t.tag):
               rang[t.tag]=(n,n)
               acc.append(t.tag)
            else:
               (mi,ma)=rang[t.tag]
               rang[t.tag]=(min(mi,n),max(ma,n))
         except:
            pass

      if 'name' in T.attrib:
         names[i]=T.attrib['name']

      for e in T.findall("./general/shortname"):
         names[i]=e.text
         break

   Res=[[k]+[l[k] if k in l else '' for l in L] for k in acc]
   names=['']+names
   length=[1]*len(names)
   length=[max(n,len(s)) for (n,s) in zip(length,names)]

   for r in Res:
      length=[max(n,len(s)) for (n,s) in zip(length,r)]

   mklin=lambda L:'| '+' | '.join(L)+' |'
   fmt=lambda (s,n):(n-len(s))*' '+s
   lfmt=lambda (s,n):s+(n-len(s))*' '

   def emph(s,(mi,ma)):
      if s=='':
         return "_"
      if mi!=ma:
         if float(s)==mi:
            return "_"+s+"_"
         elif float(s)==ma:
            return "**"+s+"**"
      return s

   print mklin(map(fmt,zip(names,length)))
   print mklin(['-'*(n-1)+ ('-' if i==0 else ':') for i,n in enumerate(length)])
   for r in Res:
      r=[r[0].replace("_"," ")]+[emph(k,rang[r[0]]) for k in r[1:]]
      print mklin([fmt(x) if i>0 else lfmt(x) for i,x in enumerate(zip(r,length))])

if __name__ == '__main__':
   if '-h' in argv[1:] or '--help' in argv[1:] or len(argv)<2:
      print "usage:",argv[0],'<outfitname1.xml> ...'
   else:
      main([f for f in argv[1:] if f.endswith(".xml")])

