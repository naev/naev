#!/usr/bin/python

import os
from glob import glob
from sys import argv


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
            print result[i],result[i+1]


