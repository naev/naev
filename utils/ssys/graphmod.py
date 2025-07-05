# python3

from sys import stdin
from geometry import vec

class _pos(dict):
   aux = {}
   def output( self ):
      for k, v in self.items():
         if k[0] != '_':
            v = round(v, 9)
            if k in self.aux:
               crt_aux = self.aux[k]
            else:
               crt_aux = []
            print(' '.join([k, str(v[0]), str(v[1])] + crt_aux))

class _jmp(dict):
   def output( self ):
      for i, l in self.items():
         for j, f in l:
            print(' '.join([i, j] + f))

   def __missing__( self, elt ):
      self[elt] = []
      return self[elt]

class _graph():
   quiet = False

   def __init__( self ):
      self.pos=_pos()
      self.jmp=_jmp()

   def silence( self ):
      self.quiet = True

   def __del__( self ):
      if not self.quiet:
         self.pos.output()
         self.jmp.output()
         for i in stdin:
            pass
         print()

graph = _graph()
sys_pos = graph.pos
sys_jmp = graph.jmp
no_graph_out = lambda:graph.silence()

for inp in stdin:
   if (line := inp.strip()) != '':
      l = line.split(' ')
      try:
         bname, x, y = tuple((l[:3]))
         sys_pos[bname] = vec(x, y)
         sys_pos.aux[bname] = l[3:]
         continue
      except:
         pass
      bname1, bname2 = tuple(l[:2])
      sys_jmp[bname1].append((bname2, l[2:]))
   else:
      break
