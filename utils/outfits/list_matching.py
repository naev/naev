#!/usr/bin/python

import os
from glob import glob
from sys import argv,stderr


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

if __name__=="__main__":
   if len(argv)!=2 or '-h' in argv or '--help' in argv:
      print >>stderr,"usage:",argv[0].split('/')[-1],"<path>"
   else:
      PATH=argv[1]
      result = [y for x in os.walk(PATH) for y in glob(os.path.join(x[0], '*.xml'))]
      result.sort()
      result=['']+result+['']
      for i in range(1,len(result)-2):
         if prefers(result[i],result[i+1],result[i-1]) and prefers(result[i+1],result[i],result[i+2]):
            if confirm(result[i],result[i+1]):
               print result[i],result[i+1]
            else:
               pass
               #print >>stderr,"\033[31mUnconfirmed pair left out\033[0m",result[i],result[i+1]


