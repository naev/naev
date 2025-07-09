#!/usr/bin/env python3

from sys import stdin, stderr, argv, exit
from geometry import vec

if __name__ == '__main__':
   if '-h' in argv[1:] or '--help' in argv[1:]:
      stderr.write(
         'usage  ' + argv[0].split('/')[-1] + '\n'
         '   This is intended as a module to build graphmods,\n'
         '   i.e. programs reading a graph on stdin and outputting a modified\n'
         '   version of the graph on stdout.\n'
         '   Calling this module as an executable works as a NO-OP graphmod,\n'
         '   i.e. it outputs the same exact graph that was in input.\n'
         '   However, this has a latching effect: no output is sent until all\n'
         '   the input has been read.\n'
      )
      exit(0)

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
         try:
            self.pos.output()
            self.jmp.output()
            for i in stdin:
               pass
            print()
         except KeyboardInterrupt:
            exit(0)
         except BrokenPipeError:
            stderr.write('Broken output pipe. Bye !\n')
            exit(0)
         except:
            pass

graph = _graph()
sys_pos = graph.pos
sys_jmp = graph.jmp
no_graph_out = lambda:graph.silence()

try:
   for inp in stdin:
      if (line := inp.strip()) != '':
         l = line.split(' ')
         try:
            bname, x, y = tuple((l[:3]))
            sys_pos[bname] = vec(x, y)
            sys_pos.aux[bname] = l[3:]
            success = True
         except:
            success = False

         if not success:
            bname1, bname2 = tuple(l[:2])
            sys_jmp[bname1].append((bname2, l[2:]))
      else:
         break
except KeyboardInterrupt:
   exit(0)
except BrokenPipeError:
   stderr.write('Broken input pipe. Bye !\n')
   exit(0)

if len(sys_pos) == 0 or len(sys_jmp) == 0:
   stderr.write('Empty input ! Bye !\n')
   exit(-1)
