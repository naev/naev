#!/usr/bin/python


from sys import argv,stderr
import xml.etree.ElementTree as ET


virtual_edges=[('flow','basel',2),('deneb','booster',1.5)]
heavy_virtual_edges=[
   ('nirtos','protera'),
   #("scholzs_star",'hystera'),('apik','ekta'),('apik','hystera'),("apik",'telika'),
   ('protera','sagittarius'),("baitas","tasopa"),('tasopa','veses'),('alpha_centauri','tasopa'),('baitas','padonia'),
   ('ngc1098','nunavut'),('ngc8338','volus'),('tobanna','brumeria'),('rotide','tide'),
   ('vean','haered'),('nava','flow'),
   ('padonia','basel'),
   ('griffin','pastor'),
   ('ngc2948','ngc9017'),
   ('levo','qellan'),
   ('nixon','gyrios'),
   ('ngc7078','anubis_black_hole'),
   ('suk','oxuram'),
   ('syndania','stint'),
   ('defa','taiomi'),('titus','solene'),('titus','diadem'),
   ('pike','kraft'),
   ('undergate','ulysses'),
   ('ngc20489','monogram'),
   ('anrique','adraia'),
   ('andee','chraan'),
   ('trohem','tepdania')
]


def xml_files_to_graph(args):
   name2id=dict()
   name,acc,pos,tradelane=[],[],[],set()

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

      for e in T.findall("tags/tag"):
         if e.text=='tradelane':
            tradelane.add(basename)
            break

      name2id[name[-1]]=basename
      acc.append([])
      count=1
      for e in T.findall("./jumps/jump"):
         try:
            acc[-1].append((e.attrib['target'],False))
            for f in e.findall("hidden"):
               acc[-1][-1]=(acc[-1][-1][0],True)
               break
         except:
            print >>stderr,"no target defined in",args[i],"jump#"+str(count)
      count+=1

   n2i=lambda x:name2id[x]
   ids=map(n2i,name)
   acc=map(lambda L:[(n2i(x),y) for (x,y) in L],acc)

   return dict(zip(ids,name)),dict(zip(ids,acc)),dict(pos),tradelane


def main(args,fixed_pos=False):
   V,E,pos,tl=xml_files_to_graph(args)

   print 'graph g{'
   print '\tepsilon=0.0000001'
   print '\tmaxiter=1000'
   #print '\tDamping=0.5'
   #print '\tmode=ipsep'

   #1inch=72pt
   if fixed_pos:
      fact=72*3/2
      print '\toverlap=true'
   else:
      fact=72*4/3
      print '\toverlap=false'  #'\toverlap=voronoi'
   print '\tinputscale='+str(fact)
   print '\tnotranslate=true' # don't make upper left at 0,0
   print '\tnode[fixedsize=true,shape=circle,color=white,fillcolor=grey,style="filled"]'
   print '\tnode[width=0.5]'
   print '\tedge[len=1.0]'
   print '\tedge[weight=100]'

   if fixed_pos:
      print '\tnode[pin=true]'

   for i in V:
      if E[i]!=[]: # Don't include disconnected systems
         s='\t"'+i+'" [label="'+V[i].replace('-','- ').replace(' ','\\n')+'"'
         #s='\t"'+i+'" [label=""'
         try:
            (x,y)=pos[i]
            s+=';pos="'+str(x)+','+str(y)+'"'
         except:
            pass
         print s+']'
         for dst,hid in E[i]:
            if i in tl and dst in tl:
               suff="[style=bold;penwidth=4.0]"
            elif hid:
               suff="[style=dotted;pendwidth=2.5]"
               #suff="[color=red]"
            else:
               suff=""
            oneway=i not in map(lambda (x,y):x,E[dst])
            if oneway:
               print '\t"'+i+'"->"'+dst+'"'+suff
            elif i<dst:
               print '\t"'+i+'"--"'+dst+'"'+suff

   if not fixed_pos:
      """
      # Transiticity edges: appears to have no effect. Why ???
      trans=set()

      for v in V:
         for i,(a,_h) in enumerate(E[v]):
            for j in range(i):
               b,_h2=E[v][j]
               if a<b:
                  if(a,b) not in trans:
                     trans.add((a,b))
               else:
                  if(b,a) not in trans:
                     trans.add((b,a))
      for v in V:
         for i,(a,_h) in enumerate(E[v]):
            if (a,v) in trans:
               trans.remove((a,v))
            if (v,a) in trans:
               trans.remove((v,a))
            
      print '\tedge[len=2.0;weight=50]'
      print '\tedge[style="invis"]'
      for (u,v) in trans:
         print '\t"'+u+'"--"'+v+'"'
      """

      print '\tedge[len=1.0;weight=100]'
      print '\tedge[style="dashed";color=grey;pendwidth=1.5]'
      for (f,t) in heavy_virtual_edges:
         print '\t"'+f+'"--"'+t+'"'

      print '\tedge[weight=1]'
      for (f,t,l) in virtual_edges:
         print '\t"'+f+'"--"'+t+'" [len='+str(l)+']'

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


