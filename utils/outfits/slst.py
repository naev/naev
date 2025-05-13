#!/usr/bin/env python3

class Slst(list):
   def __call__(self, st):
      for T in self:
         (s,t,c) = ((T)+(None,None))[:3]
         if t == None:
            t = ''
         if c is None:
            st = st.replace(s, t)
         elif c<0:
            st = ((st[::-1]).replace(s[::-1], t[::-1], -c))[::-1]
         else:
            st = st.replace(s, t, c)
      return st
   def __mul__(self, other):
      for i in range(len(self)):
         self[i] = (self[i][0], other(self[i][1]))
