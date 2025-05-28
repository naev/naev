#!/usr/bin/env python3


from ssys import vec


def median(L):
   # init pos with gravity center
   pos = vec()
   count = 0
   for p in L:
      pos += p
      count += 1
   pos /= 1.0*count

   for i in range(10):
      print('pos:'+str(round(pos,4)))
      maxi = 0.0
      for p in L:
         l = (pos-p).size()
         if l>maxi:
            maxi = l

      acc = vec()
      for p in L:
         v = p-pos
         acc += v.normalize(-((maxi/v.size())**2))

      pos += acc/(2**(i+3))
   return pos

"""
L=[(0.0,0.0), (1.0,1.0), (1.0, 0.0)]

median([vec(*t) for t in L])
"""

