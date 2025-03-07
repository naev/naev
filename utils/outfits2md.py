#!/usr/bin/python

from sys import argv,stderr
import xml.etree.ElementTree as ET


def main(args,gith=False,ter=False):
   names=['']*len(args)
   L=[dict() for a in args]
   rang=dict()
   acc=[]

   for i in range(len(args)):
      T=ET.parse(args[i]).getroot()
      for t in T.iter():
         try:
            n=abs(float(t.text))
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
   length=[4]*len(names) # :--- at least 3 - required

   if not gith:
      length=[max(n,len(s)) for (n,s) in zip(length,names)]

      for r in Res:
         length=[max(n,len(s)) for (n,s) in zip(length,r)]

   if ter:
      Sep,Lm,Rm,LM,RM='\033[34m|\033[37m',"\033[31m","\033[37m","\033[32m","\033[37m"
      mk_pad =lambda i,n:"\033[34m"+n*'-'+"\033[0m"
      leng=lambda x:len(x)- (10 if x!='' and x[0]=='\033' else 0)
   else:
      Sep,Lm,Rm,LM,RM='|',"_","_","**","**"
      mk_pad =lambda i,n:'-'*(n-1)+('-' if i==0 else ':')
      leng=len

   mklin=lambda L:Sep+' '+(' '+Sep+' ').join(L)+' '+Sep
   fmt=lambda (s,n):(n-leng(s))*' '+s
   lfmt=lambda (s,n):s+(n-leng(s))*' '

   def emph(s,(mi,ma)):
      if s=='':
         return "_"
      elif mi!=ma:
         if mi<0 and ma<0: #if all are negative, assume the lower the better
            mi,ma=ma,mi
         if float(s)==mi:
            return Lm+s+Rm
         elif float(s)==ma:
            return LM+s+RM
      return s

   print
   print mklin(map(fmt,zip(names,length)))
   print mklin([mk_pad(i,n) for i,n in enumerate(length)])
   for r in Res:
      r=[r[0].replace("_"," ")]+[emph(k,rang[r[0]]) for k in r[1:]]
      print mklin([fmt(x) if i>0 else lfmt(x) for i,x in enumerate(zip(r,length))])

if __name__ == '__main__':
   if '-h' in argv[1:] or '--help' in argv[1:] or len(argv)<2:
      print "usage:",argv[0],'[-g|-t] <outfitname1.xml> ...'
      print "By default, outputs text aligned markdown table comparing the outfits resp. values."
      print "The options improve your confort in certain use cases:"
      print "  -g  unaligned (therefore smaller) valid github md, for use in posts."
      print "  -c  colored terminal output."
   else:
      gith,ter="-g" in argv[1:],"-c" in argv[1:]
      if gith and ter:
         gith=ter=False
         print >>stderr,"Ignored incompatible -g and -c."

      ign=[f for f in argv[1:] if f not in ["-g","-c"] and not f.endswith(".xml")]
      if ign!=[]:
         print >>stderr,'Ignored: "'+'", "'.join(ign)+'"'

      main([f for f in argv[1:] if f.endswith(".xml")],gith,ter)

