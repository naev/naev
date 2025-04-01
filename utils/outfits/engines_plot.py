#!/usr/bin/env python3

import os
script_dir = os.path.dirname( __file__ )
util_dir = script_dir
engine_dir = os.path.join( script_dir, '..', '..' , 'dat' , 'outfits' , 'core_engine')

from glob import glob
import stat

from sys import path,stderr,argv
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
         acc.append((max_sp(d['speed'],d['accel']),d['speed']))
   L=list(set(acc))
   return sorted(L,reverse=True)
   
def main():
   lines=['unicorp','tricon',"nexus","melendez"]

   bas=os.path.splitext(os.path.basename(__file__))[0]
   bas=os.path.join('.',bas)
   dat=bas+'.dat'
   fp=open(dat,"wt")

   pstr=lambda t:str(t[0])+' '+str(t[1])
   for i,t in enumerate(zip(*tuple(map(mkline,lines)))):
      fp.write(' '.join([str(i+1)]+list(map(pstr,list(t))))+'\n')

   fp.close()
   stderr.write('<'+bas+'.dat>\n')

   plt=bas+'.plot'
   fp=open(plt,"wt")
   fp.write("""#!/usr/bin/gnuplot

   set terminal pngcairo size 900,600 enhanced
   """)
   fp.write('set output "'+bas+'.png"\n')
   fp.write("""set key outside
   set termoption dashed
   set logscale y sqrt(sqrt(2))
   set grid
   """)

   def fmt(dat,off,i,l):
      sp=" (drift)" if off==1 else ""
      w="linespoint" if off==0 else "lines"
      return '\t"'+dat+'" using 1:'+str(2*i+2+off)+' w '+w+' t "'+l+sp+'" linecolor '+str(i+1)

   fp.write('plot\\\n')
   fp.write(',\\\n'.join([fmt(dat,0,*t) for t in enumerate(lines)]))
   fp.write(',\\\n')
   fp.write(',\\\n'.join([fmt(dat,1,*t) for t in enumerate(lines)]))
   fp.write('\n')
   fp.close()

   stderr.write('<'+bas+'.plot>\n')

   current_permissions = stat.S_IMODE(os.lstat(plt).st_mode)
   os.chmod(plt, current_permissions | stat.S_IXUSR)

   os.system(plt)
   stderr.write('<'+bas+'.png>\n')

if __name__=='__main__':
   if len(argv)>1:
      stderr.write("usage: "+os.path.basename(__file__)+"\n")
      stderr.write("Produces a plot file, a dat file, and the resulting png where you stand.\n")
   else:
      main()

