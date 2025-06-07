#!/usr/bin/env python3

if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')


from sys import argv, stderr

from sys2graph import xml_files_to_graph


del_edges = [
   #('titus', 'vedalus'),
   #('kelvos', 'mason'),
   #('khaas', 'diadem'),
]

del_edges = set(del_edges + [(j, i) for (i, j) in del_edges])

anbh = [ 'ngc11935', 'ngc5483', 'ngc7078', 'ngc7533', 'octavian',
   'copernicus', 'ngc13674', 'ngc1562', 'ngc2601', ]

# In the form: (from, to, length, visible)
#  - length is 1 if False, None or missing
#  - visible is False if missing
virtual_edges = [
   #('khaas', 'vedalus', False, True),
   #('andres', 'mason', False, True),
   ('flow', 'basel', 2),
   ('deneb', 'booster', 1.5),
   ('ngc4746', 'logania'),
   #('akodu', 'kenvis'),
   #('tau_ceti', 'sigur'), ('tepvin', 'carrza'),
   ('thirty_stars', 'thorndyke'),
   ('herakin', 'duros'), ('rauthia', 'tide'),
   ('hekaras', 'eneguoz'), ('seifer', 'rei'),
   ('basel', 'octantis'), ('sagittarius', 'baitas'),
   ('baitas', 'tasopa'), ('percival', 'jommel'),
   ('flow', 'katami'), ('nava', 'flow'),
   ('katami', 'eisenhorn'), ('vean', 'basel'),
   ('alpha_centauri', 'tasopa'),('syndania', 'padonia'),
   ('veses', 'protera'), ('syndania', 'stint'),
   ('sagittarius', 'alpha_centauri'), ('protera', 'scholzs_star'),
   ('ngc18451', 'felzen'), ('ngc6057', 'xeric'),
   ('kiwi', 'suna'), ('ngc1098', 'westhaven'),
   ('ngc7061', 'kansas'), ('niger', 'kyo'),
   ('willow', 'palovi'), ('margarita', 'narousse'),
   ('porro', 'modus_manis'), ('suna', 'vanir'),
   ('tobanna', 'brumeria'),('rotide', 'tide'),
   ('padonia', 'basel'), ('ogat', 'wochii'),
   ('griffin', 'pastor'), ('ngc2948', 'ngc9017'),
   ('ngc4131', 'neexi'), ('c59', 'c14'),
   ('c43', 'c28'), ('levo', 'qellan'),
   ('nixon', 'gyrios'), ('suk', 'oxuram'),
   ('defa', 'taiomi'), ('titus', 'solene'), ('titus', 'diadem'),
   ('pike', 'kraft'), ('undergate', 'ulysses'),
   ('ngc20489', 'monogram'), ('anrique', 'adraia'),
   ('andee', 'chraan'), ('trohem', 'tepdania'),
   ('ngc14479', 'zintar'), ('pudas', 'fried'),
   ('blunderbuss', 'darkstone'), ('ekkodu', 'tarsus'),
   ('ivella', 'jommel'), ('starlight_end', 'possum'),
   ('ngc8338', 'unicorn'), ('ngc22375', 'undergate'),
]

prv, prvj  = None, None
for j, i in enumerate(anbh):
   if prv is None:
      prv = i
   else:
      if prvj is not None:
         virtual_edges.append(('_'+str(prvj),    '_'+str(j)))
      prvj = j
      virtual_edges.append(('anubis_black_hole', '_'+str(j)))
      virtual_edges.append(('_'+str(j),                 prv))
      virtual_edges.append(('_'+str(j),                   i))
      prv = None

if prv is not None:
   virtual_edges.append((prv,                           i))
   virtual_edges.append(('_'+str(prvj),   '_'+str(prvj+2)))
   virtual_edges.append(('_'+str(prvj+2),             prv))
   virtual_edges.append(('_'+str(prvj+2),      '_'+str(1)))

virtual_edges = [(t + (False, False))[:4] for t in virtual_edges]
virtual_edges = [t[:2]+((t[2] or 1.0),)+t[3:] for t in virtual_edges]

already = set()
for i in virtual_edges:
   if i in already:
      stderr.write(str(i) + ' appears twice in virtual_edges list !\n')
   else:
      already.add(i)

def main( args, fixed_pos = False, color = False ):
   V, pos, E, tl, colors = xml_files_to_graph(args, color)
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

   virt_v = set()
   for e in virtual_edges:
      if not e[3]:
         virt_v.update(set([x for x in e[:2] if x not in V]))

   if not fixed_pos:
      for i in sorted(virt_v):
         if i not in V:
            print('\t"' + i + '" [label="",style=invis]')

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
            cols = [int(255.0*(f/3.0+2.0/3.0)) for f in colors[i]]
            rgb = ''.join([('0'+(hex(v)[2:]))[-2:] for v in cols])
            s += ';fillcolor="#'+rgb+'"'

         if i == 'sol':
            s += ';color=red'

         print(s + ']')
         for dst, hid in E[i]:
            suff = []
            if (i, dst) in del_edges:
               if fixed_pos:
                  suff.append('color="red"')
               else:
                  continue
            if i in tl and dst in tl:
               suff.extend(['style=bold', 'penwidth=4.0'])
            elif hid:
               suff.extend(['style=dotted', 'penwidth=2.5'])

            suff = '[' + ';'.join(suff) + ']' if suff != [] else ''

            oneway = i not in map(lambda t:t[0], E[dst])
            edge = '->' if oneway else '--'
            if oneway or i<dst:
               print('"'.join(['\t', i, edge, dst, suff]))

   print('\tedge[len=' + str(reflen) + ']')
   print('\tedge[style="dashed";color="grey";penwidth=1.5]')
   for (f, t, l, v) in virtual_edges:
      prop = []

      if v:
         prop.extend(['style="normal"', 'color="green"'])
      elif not fixed_pos:
         if f in virt_v or t in virt_v:
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
   if '-h' in argv[1:] or '--help' in argv[1:] or len(argv)<2:
      print('usage: ', argv[0], '[-c]', '[-k]', '<sys1.xml>', '...')
      print('Outputs the graph in dot format.')
      print('If -c is set, use faction colors (slower).')
      print('If -k is set, the nodes have the keep_position marker.')
      print('Examples:')
      print('  > ./utils/sys2dot.py dat/ssys/*.xml -k | neato -Tpng > before.png')
      print('  > ./utils/sys2dot.py dat/ssys/*.xml | neato -Tpng > after.png')
      print('  > ./utils/sys2dot.py dat/ssys/*.xml | neato | tee after.dot |  ./utils/sys/dot2sys.py')
      print('  > display before.png after.png')
   else:
      if keep := '-k' in argv:
         argv.remove('-k')

      if color := '-c' in argv:
         argv.remove('-c')

      if (ign := [f for f in argv[1:] if not f.endswith('.xml')]) != []:
         stderr.write('Ignored: "' + '", "'.join(ign) + '"\n')

      main([f for f in argv[1:] if f.endswith('.xml')], keep, color)
