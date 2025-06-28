# python3

from sys import stdin
from geometry import vec

class _pos(dict):
   def __del__( self ):
      for k, v in self.items():
         if k[0] != '_':
            v = round(v, 9)
            print(k, v[0], v[1])

class _E(dict):
   def __del__( self ):
      for i, l in self.items():
         for j, f in l:
            print(i, j, f)

sys_pos = _pos()
sys_jmp = _E()

for inp in stdin:
   if (line := inp.strip()) != '':
      try:
         bname, x, y, aux = tuple((line.split(' ') + ['']) [:4])
         sys_pos[bname] = vec(x, y)
         continue
      except:
         pass
      bname1, bname2, tag = tuple((line.split(' ',2) + [None]) [:3])
      if bname1 not in sys_jmp:
         sys_jmp[bname1] = []
      sys_jmp[bname1].append((bname2, tag))
