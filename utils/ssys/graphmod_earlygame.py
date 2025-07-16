#!/usr/bin/env python3


if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

from sys import stderr, argv, exit

if argv[1:] != []:
   stderr.write(
      'usage: ' + argv[0].split('/')[-1] + '\n'
      '  Reads a graph file on stdin, outputs a graph on stdout.\n'
      '  Keeps only systems reachable in early game :\n'
      '   - the player starts at Delta Polaris.\n'
      '   - the player can only refuel in friendly systems:\n'
      '      - Empire\n'
      '      - Soromid\n'
      '      - Dvaered\n'
      '      - Za\'lek\n'
      '      - Frontier\n'
      '      - Trader Society\n'
      '      - Independant\n'
      '   - the player can cross at most one non-friendly system.\n'
      '   - the player takes only non-hidden two-ways jumps.\n'
   )
   exit(0)

from graphmod import ssys_pos, ssys_jmp
from graph_vaux import faction_color, ssys_color, ssys_nebula

friendly = {
   'empire', 'zalek', 'dvaered', 'sirius', 'soromid',
   'frontier', 'independent', 'traders_society',
}
friendly_color = { faction_color[i] for i in friendly }

hostile = { 'flf', 'pirate' }
hostile_color = { faction_color[i] for i in hostile }

def get_col( sys ):
   f = ssys_color(ssys_pos, sys)
   if f in faction_color:
      return faction_color[f]
   else:
      return f

def is_friendly( sys ):
   return get_col(sys) in friendly_color

def is_hostile( sys ):
   return get_col(sys) in hostile_color or (ssys_nebula(ssys_pos, sys) or 0.0) > 5.0

def non_hidden( sys ):
   return { n for n, t in ssys_jmp[sys] if 'hidden' not in t }

def insert( sys, keep = {} ):
   if sys in keep:
      return
   keep[sys] = not is_hostile(sys)
   N = (n for n in non_hidden(sys) if sys in non_hidden(n))
   # can refuel and explore
   if is_friendly(sys):
      for n in N:
         insert(n, keep)
   # can explore
   elif not is_hostile(sys):
      for n in N:
         if is_friendly(n):
            insert(n, keep)
   return keep

keep = insert('delta_polaris')

for i in [i for i in ssys_pos if i not in keep]:
   del ssys_pos[i]

for i in [j for j in ssys_jmp]:
   if i not in keep:
      del ssys_jmp[i]
   else:
      ssys_jmp[i] = [(a, b) for (a, b) in ssys_jmp[i] if a in keep and 'hidden' not in b]
