#!/usr/bin/env python

import os
script_dir = os.path.dirname( __file__ )
util_dir = script_dir
engine_dir = os.path.join( script_dir, '..', '..' , 'dat' , 'outfits' , 'core_engine')

from glob import glob
import stat

from sys import path,stderr
path.insert(0,util_dir)

from getconst import PHYSICS_SPEED_DAMP
import outfit


def iter_line(line):
   for d in ['small','medium','large']:
      for i in glob(os.path.join( engine_dir, d , line+'*.xml')):
         yield i

def max_sp(sp,ac):
   sp=float(sp)
   ac=float(ac)
   return round(sp+ac/PHYSICS_SPEED_DAMP,2)

def mkline(line):
   acc=[]
   for f in iter_line(line):
      o=outfit.outfit(f)
      if(o):
         d=o.to_dict()
         acc.append(max_sp(d['speed'],d['accel']))
   return sorted(acc,reverse=True)

lines=['unicorp','tricon',"nexus"]

bas=os.path.splitext(__file__)[0]
dat=bas+'.dat'
fp=open(dat,"wt")

for i,t in enumerate(zip(*tuple(map(mkline,lines)))):
   fp.write(' '.join(map(str,[i+1]+list(t)))+'\n')

fp.close()

plt=bas+'.plot'
fp=open(plt,"wt")
fp.write("""#!/usr/bin/gnuplot

set terminal pngcairo size 800,600 enhanced
""")
fp.write('set output "'+bas+'.png"\n')
fp.write("""set key outside
set termoption dashed
set logscale y sqrt(sqrt(2))
set grid
""")

fp.write('plot\\\n')
fp.write(',\\\n'.join(['\t"'+dat+'" using 1:'+str(i+2)+' with linespoints t "'+l+'"' for i,l in enumerate(lines)]))
fp.write('\n')
fp.close()

current_permissions = stat.S_IMODE(os.lstat(plt).st_mode)
os.chmod(plt, current_permissions | stat.S_IXUSR)

os.system(plt)
stderr.write('<'+bas+'.png>\n')


