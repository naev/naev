#!/usr/bin/env python3

if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')


from sys import argv, stderr
from graph_vaux import color_values


from graphmod import sys_pos as pos, sys_jmp as E
E.silence()
pos.silence()

def main( color = False, fixed_pos = False ):
   if color:
      nebula = { k for k, v in pos.aux.items() if v[:1] == ['nebula'] }
      colors = { k: color_values[(v+['default'])[0]] for k, v in pos.aux.items()}
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
            continue
         elif 'tradelane' in aux:
            suff.extend(['style=bold', 'penwidth=4.0'])
         elif 'hidden' in aux:
            suff.extend(['style=dotted', 'penwidth=2.5'])

         suff = '[' + ';'.join(suff) + ']' if suff != [] else ''
         oneway = i not in map(lambda t:t[0], E[dst])
         edge = '->' if oneway else '--'
         if oneway or i<dst:
            print('"'.join(['\t', i, edge, dst, suff]))

   print('\tedge[len=' + str(reflen) + ']')
   print('\tedge[style="dashed";color="grey";penwidth=1.5]')
   for f, k in E.items():
      for t, aux in k:
         if 'virtual' not in aux:
            continue
         try:
            l = float(aux[0])
         except:
            l = 1.0

         prop = []
         if not fixed_pos:
            if f[0] == '_' or t[0] == '_':
               prop.append('style="invis"')
         else:
            continue

         if l != 1.0:
            prop.append('len='+str(l*reflen))

         prop = ';'.join(prop)
         if prop != '':
            prop = ' [' + prop + ']'
         print('\t"' + f + '"--"' + t + '"' + prop)
   print('}')

if __name__ == '__main__':
   if '-h' in argv[1:] or '--help' in argv[1:] or len(argv)>3:
      print('usage: ', argv[0], '[-c]', '[-k]')
      print('  Outputs the graph in dot format.')
      print('  By default, interprets the vertex aux as the name.')
      print('  If -c is set, interprets the first aux field as color.')
      print('  If -k is set, the nodes have the keep_position marker.')
      print('Examples:')
      print('  > ./utils/ssys/ssys2dot.py -k | neato -Tpng > before.png')
      print('  > ./utils/ssys/ssys2dot.py | neato -Tpng > after.png')
      print('  > display before.png after.png')
   else:
      if color := '-c' in argv:
         argv.remove('-c')

      if keep := '-k' in argv:
         argv.remove('-k')

      if argv[1:] != []:
         stderr.write('Ignored: "' + '", "'.join(argv[1:]) + '"\n')

      main(color, keep)
