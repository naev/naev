#!/usr/bin/python

import os
from os import path
from glob import glob
from sys import argv,stderr


def get_path(s):
   s=path.dirname(s)

   if s=='':
      return ''
   else:
      return s+path.sep

def longest_prefix_len(s,t):
   count=0
   for x,y in zip(s,t):
      if x!=y:
         break
      count+=1
   return count

def prefers(s,cand1,cand2):
   n1=longest_prefix_len(s,cand1)
   n2=longest_prefix_len(s,cand2)
   return n1>n2

def confirm(cand1,cand2):
   for i in range(2):
      n=longest_prefix_len(cand1,cand2)
      cand1,cand2=cand1[n:],cand2[n:]
      cand1=''.join(reversed(cand1))
      cand2=''.join(reversed(cand2))

   # For e.g. light Vs ultralight
   if cand1=='' or cand2=='':
      return True

   try:
      # When they differ only by a number
      n1,n2=int(cand1),int(cand2)
      return True
   except:
      return False

def process(thepath,f1,f2):
   inpath=f1.rsplit('/',1)[0]+'/'
   outpath=get_path(f1.replace('/core_','/multicore/core_',1))
   #print "echo","'"+path.basename(f1),path.basename(f2)+"'"
   cmd1=thepath+"outfits2mvx.py"+' '+f1+' '+f2
   cmd2=thepath+'mvx2xmllua.py '
   print 'NAM=`('+cmd1+' 2>/dev/null | '+cmd2+' 1>/dev/null) 2>&1`'
   print 'if [ "$NAM" != "" ];then echo \\"$NAM\\"'
   print '\t'+cmd1+' | tee '+outpath+'${NAM}.mvx | '+cmd2+' 2>/dev/null > '+outpath+'${NAM}.xml'
   print '\t'+thepath+"deprecate_outfit.py",inpath+"${NAM}.xml"
   print 'fi'

if __name__=="__main__":
   if len(argv)!=2 or '-h' in argv or '--help' in argv:
      print >>stderr,"usage:",argv[0].rsplit('/',1)[-1],"<path>"
   else:
      PATH=argv[1]
      if '/' in argv[0]:
         crt_path=argv[0].rsplit('/',1)[0]+'/'
      else:
         crt_path=''

      print "#!"
      result = [y for x in os.walk(PATH) for y in glob(os.path.join(x[0], '*.xml'))]
      result.sort()
      result=['']+result+['']
      done=set()
      for i in range(1,len(result)-2):
         if prefers(result[i],result[i+1],result[i-1]) and prefers(result[i+1],result[i],result[i+2]):
            if confirm(result[i],result[i+1]):
               done.add(result[i])
               done.add(result[i+1])
               process(crt_path,result[i],result[i+1])
            else:
               pass
               #print >>stderr,"\033[31mUnconfirmed pair left out\033[0m",result[i],result[i+1]
      for alone in result[1:-1]:
         if alone not in done:
            process(crt_path,alone,'')
