#!/usr/bin/env python3

decorators = {
   'blackhole': 'anubis_black_hole',
   'empire': 'gamma_polaris',
   'dvaered': 'beeklo',
   'sirius': 'adraia',
   'soromid': 'pisces_prime',
   'zalek' : 'zalek',
   'frontier': 'chraan',
   'proteron': 'korifa',
   'nebula': 'midoros'
}

if __name__ == '__main__':
   from ssys import getpath, PATH, naev_xml
   from graphmod import ssys_pos, no_graph_out
   no_graph_out()
   DIR = getpath(PATH, 'map_decorator')
   for i in decorators:
      fnam = getpath( DIR, i + '.xml')
      v = ssys_pos[decorators[i]]
      T = naev_xml(fnam)
      T['decorator']['$x'] = v[0]
      T['decorator']['$y'] = v[1]
      T.save()
