#!/usr/bin/env python3

class Slst(list):
   def __call__(self, s):
      for (i,j) in self:
         if i[0] == '^':
            s = s.replace(i[1:], j, 1)
         elif i[-1] == '$':
            n = s.rfind(i[:-1])
            if n!=-1:
               s = s[:n] + j + s[n+len(i)-1:]
         else:
            s = s.replace(i, j)
      return s
   def __mul__(self, other):
      for i in range(len(self)):
         self[i] = (self[i][0], other(self[i][1]))
