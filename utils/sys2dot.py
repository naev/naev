#!/usr/bin/python

from sys import argv,stderr
import xml.etree.ElementTree as ET


def xml_files_to_graph(args):
   name2id=dict()
   name,acc,pos=[],[],[]

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
         print >>stderr,"no name defined in",basename

      for e in T.findall("pos"):
         try:
            pos.append((basename,(e.attrib['x'],e.attrib['y'])))
         except:
            print >>stderr,"no position defined in",basename

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

   return dict(zip(ids,name)),dict(zip(ids,acc)),dict(pos)


def main(args,fixed_pos=False):
   V,E,pos=xml_files_to_graph(args)

   print 'graph g{'
   print '\tepsilon=0.000001;overlap=false'
   #1inch=72pt
   fact=72
   print '\tinputscale='+str(fact)
   #print '\tDamping=0.5'
   #print '\tmaxiter=600'
   #print '\tmode=ipsep'
   #print '\toverlap=voronoi'
   print '\toverlap=true'
   print '\tnotranslate=true' # don't make upper left at 0,0
   print '\tnode[fixedsize=true,shape=circle,color=white,fillcolor=grey,style="filled"]'
   print '\tedge[len=1.0]'

   if fixed_pos:
      print '\tnode[pin=true]'

   for i in V:
      if E[i]!=[]: # Don't include disconnected systems
         s='\t"'+i+'" [label="'+V[i]+'"'
         try:
            (x,y)=pos[i]
            s+=';pos="'+str(x)+','+str(y)+'"'
         except:
            pass
         print s+']'
         for dst in E[i]:
            if i<dst or i not in E[dst]:
               print '\t"'+i+'"--"'+dst+'"'
   print "}"

if __name__ == '__main__':
   if '-h' in argv[1:] or '--help' in argv[1:] or len(argv)<2:
      print "usage:",argv[0],'[-k] <sys1.xml> ...'
      print 'Outputs the graph in dot format.'
      print 'If -k is set, the nodes have the keep_position marker.'
      print 'Examples:'
      print '  > ./utils/sys2dot.py dat/ssys/*.xml -k | neato -Tpng > before.png'
      print '  > ./utils/sys2dot.py dat/ssys/*.xml | neato -Tpng > after.png'
      print '  > display before.png after.png'
   else:
      keep='-k' in argv
      if keep:
         argv.remove('-k')

      ign=[f for f in argv[1:] if not f.endswith(".xml")]
      if ign!=[]:
         print >>stderr,'Ignored: "'+'", "'.join(ign)+'"'

      main([f for f in argv[1:] if f.endswith(".xml")],keep)


