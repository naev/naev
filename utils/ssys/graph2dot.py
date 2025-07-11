#!/usr/bin/env python3

if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')


from sys import argv, stderr
from graph_vaux import color_values, ssys_color, ssys_nebula


def main( color = False, fixed_pos = False ):
   from graphmod import sys_pos as pos, sys_jmp as E, no_graph_out
   no_graph_out()

   if color:
      colors = { k: color_values[ssys_color(pos, k)] for k in pos }
      nebula = { k for k in pos if ssys_nebula(pos, k) }
      V = {k:' '.join(l[1:]) for k, l in pos.aux.items()}
   else:
      V = {k:' '.join(l) for k, l in pos.aux.items()}
   print('graph g{')
   print('\tepsilon=0.000001')
   print('\tmaxiter=2000')

   # 1inch=72pt
   if fixed_pos:
      print('\tgraph [overlap=true]')
      factor = 0.7
   else:
      print('\tgraph [overlap=false]')  #'\toverlap=voronoi'
      factor = 0.7

   print('\tinputscale=72')
   print('\tnotranslate=true') # don't make upper left at 0,0
   print('\tnode[fixedsize=true,shape=circle,color=white,fillcolor=grey,style="filled"]')
   reflen = 0.5
   print('\tnode[width=0.5]')
   print('\tedge[len='+str(reflen)+']')

   if fixed_pos:
      print('\tnode[pin=true]')

   for i in V:
      if i[0] == '_' and fixed_pos:
         continue
      # Don't include disconnected systems
      if E[i] != [] or fixed_pos:
         s = '\t"'+i+'" ['
         if i[0] != '_':
            (x, y) = pos[i]
            x = round(float(x)*factor, 9)
            y = round(float(y)*factor, 9)
            s += 'pos="'+str(x)+','+str(y)+('!' if fixed_pos else '')+'";'
         label = V[i]
         for t in [('-','- '), (' ','\\n'), ('Test\\nof','Test of')]:
            label = label.replace(*t)
         s += 'label="' + label + '"'

         if color:
            if i in nebula:
               s += ';fontcolor=darkred'
            cols = [int(255.0*(f/3.0+2.0/3.0)) for f in colors[i]]
            rgb = ''.join([('0'+(hex(v)[2:]))[-2:] for v in cols])
            s += ';fillcolor="#'+rgb+'"'

         if i == 'sol':
            s += ';color=red'

         print(s + ']')

   # all others are virtual
   print('\tnode [label="",style=invis]')

   for i in V:
      for dst, aux in E[i]:
         suff = []
         if 'virtual' in aux:
            continue;
         elif 'tradelane' in aux:
            suff += ['style=bold', 'penwidth=4.0']
         elif 'hidden' in aux:
            suff += ['style=dotted', 'penwidth=2.5']
         elif 'new' in aux:
            suff += ['color=green']
         elif 'fake' in aux:
            suff += ['color=red', 'weight=0']

         suff = '[' + ';'.join(suff) + ']' if suff != [] else ''
         oneway = i not in map(lambda t:t[0], E[dst])
         if oneway or i<dst:
            print('"'.join(['\t', i, '--', dst, suff]))

   print('\tedge[len=' + str(reflen) + ']')
   print('\tedge[style="dashed";color="grey";penwidth=1.5]')
   for f, k in E.items():
      for t, aux in k:
         if fixed_pos or 'virtual' not in aux:
            continue
         try:
            l = float(aux[0])
         except:
            l = 1.0

         prop = []
         if f[0] == '_' or t[0] == '_':
            prop += ['style="invis"']

         if l != 1.0:
            prop += ['len='+str(l*reflen)]

         prop = ';'.join(prop)
         if prop != '':
            prop = ' [' + prop + ']'
         print('\t"' + f + '"--"' + t + '"' + prop)
   print('}')

if color := '-c' in argv:
   argv.remove('-c')

if keep := '-k' in argv:
   argv.remove('-k')

if '-h' in argv[1:] or '--help' in argv[1:]:
   print(
      'usage: ', argv[0], '[-c]', '[-k]', '\n'
      '  Outputs the graph in dot format.\n'
      '  By default, interprets the vertex aux as the name.\n'
      '  If -c is set, interprets the first aux field as color.\n'
      '  If -k is set, the nodes have the keep_position marker.\n'
      'Examples:\n'
      '  > ./utils/ssys/ssys2dot.py -k | neato -Tpng > before.png\n'
      '  > ./utils/ssys/ssys2dot.py | neato -Tpng > after.png\n'
      '  > display before.png after.png'
   )
else:
   if argv[1:] != []:
      stderr.write('Ignored: "' + '", "'.join(argv[1:]) + '"\n')

   main(color, keep)
