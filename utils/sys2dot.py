#!/usr/bin/python

from sys import argv,stderr
import xml.etree.ElementTree as ET


def main(args):
   name2id=dict()
   name=[]
   acc=[]

   if args==[]:
      return

   for i in range(len(args)):
      basename=args[i].rsplit(".xml",1)
      if len(basename)!=2 or basename[1]!='':
         print >>stderr,"err:",args[i]
         continue

      basename=basename[0].rsplit('/',1)[-1]
      name.append(basename)
      T=ET.parse(args[i]).getroot()

      try:
         name[-1]=T.attrib['name']
      except:
         print >>stderr,"no name defined in",args[i]

      name2id[name[-1]]=basename
      acc.append([])
      count=1
      for e in T.findall("./jumps/jump"):
         try:
            acc[-1].append(e.attrib['target'])
         except:
            print >>stderr,"no target defined in",args[i],"jump#"+str(count)
      count+=1

   n2i=lambda x:name2id[x]
   ids=map(n2i,name)
   acc=map(lambda x:map(n2i,x),acc)

   V=dict(zip(ids,name))
   E=dict(zip(ids,acc))
   del ids,acc

   print 'graph g{'
   print '\tnode[fixedsize=true,shape=circle,color=white,fillcolor=grey,style="filled"]'
   print '\toverlap=false'
   for i in V:
      if E[i]!=[]:
         print '\t"'+i+'" [label="'+V[i]+'"]'
         for dst in E[i]:
            if i<dst or i not in E[dst]:
               print '\t"'+i+'"--"'+dst+'"'
   print "}"
   

if __name__ == '__main__':
   if '-h' in argv[1:] or '--help' in argv[1:] or len(argv)<2:
      print "usage:",argv[0],'<sys1.xml> ...'
   else:
      ign=[f for f in argv[1:] if not f.endswith(".xml")]
      if ign!=[]:
         print >>stderr,'Ignored: "'+'", "'.join(ign)+'"'

      main([f for f in argv[1:] if f.endswith(".xml")])

