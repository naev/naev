#!/usr/bin/env python3


lines = ['unicorp', 'tricon', 'nexus', 'melendez', 'krain', 'beat_up']


import os
script_dir = os.path.dirname( __file__ )
util_dir = script_dir
engine_dir = os.path.realpath(os.path.join( script_dir, '..', '..', 'dat', 'outfits', 'core_engine'))

from glob import glob
import stat

from sys import path, stderr, argv
path.insert(0, util_dir)

from getconst import PHYSICS_SPEED_DAMP
import outfit


def iter_line( line ):
   for d in ['small', 'medium', 'large']:
      for i in glob(os.path.join( engine_dir, d, line+'*.mvx')):
         yield i

def max_sp( sp, ac ):
   sp = float(sp)
   ac = float(ac)
   return round(sp+ac/PHYSICS_SPEED_DAMP, 2)

def mkline( line ):
   acc = []
   for f in iter_line(line):
      for fl in [False, True]:
         o = outfit.outfit(f)
         o.autostack(fl)
         d = o.to_dict()
         acc.append((max_sp(d['speed'], d['accel']), d['speed']))

   L = list(sorted(set(acc), reverse = True))

   if len(L) == 4:  # That's Krain!
      # Complete with padding
      L = L+2*[('.', '.')]

   return L

def main( ):
   bas = os.path.splitext(os.path.basename(__file__))[0]
   bas = os.path.join('.', bas)
   dat = bas+'.dat'
   fp = open(dat, 'wt')

   pstr = lambda t:str(t[0])+' '+str(t[1])
   for i, t in enumerate(zip(*tuple(map(mkline, lines)))):
      fp.write(' '.join([str(i+1)]+list(map(pstr, list(t))))+'\n')

   fp.close()
   stderr.write('<'+bas+'.dat>\n')

   plt = bas+'.plot'
   fp = open(plt, 'wt')
   fp.write("""#!/usr/bin/gnuplot\n
set terminal pngcairo transparent truecolor size 600,400 font "Helvetica,10" enhanced\n""")
   fp.write('set output "'+bas+'.png"\n')
   fp.write("""set key outside
   set termoption dashed
   set logscale y sqrt(sqrt(2))
   set style line 101 lc rgb '#808080' lt 1 lw 1
   set border 3 front ls 101
   set key textcolor rgb '#808080'

   set style line 12 lc rgb '#808080' lt 3 lw 0.8 dt ".."
   set grid xtics ytics mxtics mytics ls 12
   """)

   def fmt( dat, off, i, l ):
      if off == 0:
         #w = 'linespoint'
         w = 'lines'
         sp = ''
         lw = '0.9'
      else:
         w = 'lines'
         sp = ' (drift)'
         lw = '0.85 dt "-"'
      l = '"'+l.replace('_', ' ')+sp+'"'
      n = i+1
      if n >= 5:
         n += 1
      return '\t"'+dat+'" using 1:'+str(2*i+2+off)+' w '+w+' t '+l+' linecolor '+str(n)+' lw '+lw

   fp.write('plot\\\n')
   fp.write(',\\\n'.join([fmt(dat, i,*t) for i in range(2) for t in enumerate(lines)]))
   fp.write('\n')
   fp.close()

   stderr.write('<'+bas+'.plot>\n')

   current_permissions = stat.S_IMODE(os.lstat(plt).st_mode)
   os.chmod(plt, current_permissions | stat.S_IXUSR)

   os.system(plt)
   stderr.write('<'+bas+'.png>\n')

if __name__ == '__main__':
   if len(argv)>1:
      stderr.write('usage: '+os.path.basename(__file__)+'\n')
      stderr.write('Produces a plot file, a dat file, and the resulting png where you stand.\n')
   else:
      main()
