#!/usr/bin/env python3

decorators = {
   'blackhole': 'anubis_black_hole',
   'empire': 'gamma_polaris',
   'dvaered': 'beeklo',
   'sirius': 'eiderdown',
   'soromid': 'pisces_prime',
   'zalek' : 'zalek',
   'frontier': 'chraan',
   'proteron': 'korifa',
   'nebula': 'oriantis'
}

if __name__ == '__main__':
   from ssys import getpath, PATH, fil_ET
   from graphmod import ssys_pos as V, no_graph_out
   no_graph_out()
   DIR = getpath(PATH, 'map_decorator')
   for i in decorators:
      fnam = getpath( DIR, i + '.xml')
      T = fil_ET(fnam)
      if  ((x := T.find('x')) is not None) and ((y := T.find('y')) is not None):
         v = V[decorators[i]]
         x.text = str(v[0])
         y.text = str(v[1])
         T.write(fnam)

