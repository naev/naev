#!/usr/bin/env python3

import math
from sys import stderr,stdout
from outfit import outfit,nam2fil
from getconst import PHYSICS_SPEED_DAMP


AG_EXP  = 0.25
TURN_CT = 0.43
STD_R   = 0.12
R_MAG   = 1.5

line_stats = {
    "Tricon" : {
        "ratio" : 1.4, # 2.0 is double accel vs speed (at size 1)
        "speed_rank_offset" : 0.0,  # 0.0 indicates current speed rank
                                    # 1.0 means next speed rank (size+1);
                                    # !!! higher means slower
    },
    "Krain" : {
        "ratio" : 1.1,
        "speed_rank_offset" : -0.3, # Between this size and size-1
         "turn" : 1.1
    },
    "Nexus" : {
        "ratio" : 1.0,
        "speed_rank_offset" : +0.11, # Pretty good but slightly slower top speed than Melendez and Tricon
    },
    "Melendez" : {
        "ratio" : 0.65,
        "speed_rank_offset" : +0.07,
    },
    "Unicorp" : {
        "ratio" : 1.0,
        "speed_rank_offset" : +0.4,
    },
    "Za'lek" : { # TODO make these change over time the profile via Lua
        "ratio" : 1.1,
        "speed_rank_offset" : +0.5,
    },
    "Beat" : {
        "ratio" : 0.7,
        "speed_rank_offset" : +0.7,
    },
}

ALPHA, BETA = 1.14, 0.048

def dec_i(n):
   if n<=1:
      return 400.0
   else:
      return dec_i(n-1)/(ALPHA+BETA*(n-1))

sizes={
   "Za'lek Test Engine":2,
   "Beat Up Small Engine":1,
   "Beat Up Medium Engine":3,
   "Beat Up Large Engine":5,
   'Unicorp Falcon 1400 Engine':4,
   'Tricon Cyclone II Engine':4,
   'Unicorp Falcon 700 Engine':3,
   'Nexus Arrow 1400 Engine':4,
   'Nexus Arrow 700 Engine':3,
   'Melendez Buffalo XL Engine':4,
   'Melendez Buffalo Engine':3,
   'Beat Up Medium Engine':3,
   'Krain Patagium Engine':4,
   'Tricon Cyclone Engine':3,
   'Krain Remige Engine':5,
   'Melendez Mammoth XL Engine':6,
   'Melendez Mammoth Engine':5,
   'Melendez Old Mammoth Engine':5,
   'Unicorp Eagle 6500 Engine':6,
   'Unicorp Eagle 3000 Engine':5,
   'Beat Up Large Engine':5,
   'Tricon Typhoon Engine':5,
   'Tricon Typhoon II Engine':6,
   'Nexus Bolt 6500 Engine':6,
   'Nexus Bolt 3000 Engine':5,
   'Unicorp Hawk 360 Engine':2,
   'Unicorp Hawk 160 Engine':1,
   'Beat Up Small Engine':1,
   'Melendez Ox XL Engine':2,
   'Melendez Ox Engine':1,
   "Za'lek Test Engine":2,
   'Tricon Zephyr Engine':1,
   'Tricon Zephyr II Engine':2,
   'Nexus Dart 360 Engine':2,
   'Nexus Dart 160 Engine':1
}

def fmt(f):
   res=str(int(round(f)))

   #if f>100.0 and res[-1] in "9146" or f>200.0:
   #   res=str(int(5.0*round(f/5.0)))
   return res


def fmt_t(t):
   if t>=45:
      return fmt(t)
   else:
      res=round(2*t)/2.0
      if res==int(res):
         return str(int(res))
      else:
         return str(res)

def dec(f):
   n = math.floor(f)
   q = 1.0*f-n
   n = int(n)
   return pow(dec_i(n),1.0-q)*pow(dec_i(n+1),q)

def ls2vals(line_size):
   if line_size is None:
      return None
   (line,size) = line_size
   stats = line_stats[line]

   # Modulate full speed based on the speed_ranke_offset stat
   fullspeed = dec( size + stats["speed_rank_offset"])

   # r ranges from 15% / 2 (size 6) to 15% * 2 (size 1)
   r = STD_R * pow(2,-R_MAG*((size-1)-2.5)/5)

   # Modulate ratio based on outfit
   r *= line_stats[line]["ratio"]

   speed = fullspeed*(1.0-r)
   accel = fullspeed*r*PHYSICS_SPEED_DAMP

   turn = TURN_CT * fullspeed * pow(r/STD_R,AG_EXP)
   if "turn" in stats:
      turn*=stats["turn"]

   return {
           "speed":fmt(speed),
           "accel":fmt(accel),
           "turn":fmt_t(turn)
          }

def get_line(o):
   res=o.name().split(' ')[0]
   return res if res in line_stats else None

def get_size(o):
   res=o.name()
   if res in sizes:
      return sizes[res]
   else:
      return None

def get_line_size(o):
   l=get_line(o)
   s=get_size(o)
   if l!=None and s!=None:
      return (l,s)
   else:
      return None

out=lambda x:stdout.write(x+'\n')
err=lambda x,nnl=False:stderr.write(x+('\n' if not nnl else ''))

def apply_ls(o,ls,additional=dict()):
   sub= ls2vals(ls)
   if sub is not None:
      for k,v in additional.items():
         if k not in sub:
            sub[k]=v
      acc=[]
      for i in o:
         if i.tag in sub and i.text!=sub[i.tag]:
            acc.append((i.tag,i.text,sub[i.tag]))
            i.text=sub[i.tag]
      return acc
   return None

def main(args):
   outfits = []
   for a in args:
      o = outfit(a)
      if o is not None:
         acc=apply_ls(o,get_line_size(o))
         if acc is not None:
            err(o.fil.split('/')[-1]+': ',nnl=True)
            if acc!=[]:
               err(', '.join([i+':'+j+'->'+k for i,j,k in acc]))
               fp=open(o.fil,"w")
               o.write(fp)
               fp.close()
            else:
               err('_')
   return 0

def gen_line(lin):
   import os
   outf_dir = os.path.join( os.path.dirname( __file__ ), '..', '..', 'dat', 'outfits')
   engine_dir = os.path.join( outf_dir, 'core_engine', 'small', 'beat_up_small_engine.xml')
   o=outfit(engine_dir)
   if o is None:
      err('Dummy engine not found!')
      return 1

   for i,s in enumerate(["S1","S2","M1","M2","L1","L2"]):
      siz=i+1
      nam=lin+" "+s
      fil=nam2fil(nam+'.xml')
      o.r.attrib['name']=nam
      acc=apply_ls(o,(lin,siz),{
         'mass':str(10*siz),
         'engine_limit':str(125*2**i),
         'fuel':str(int((siz+3)*100*(2**((i-i%2)/2)))),
         'description':'TODO'
      })
      fp=open(fil,"w")
      o.write(fp)
      fp.close()
      err('<'+fil+'>')

   return 0

if __name__=="__main__":
   import argparse

   parser = argparse.ArgumentParser(
      usage=" %(prog)s (-g LINE) | [filename ...]",
      formatter_class=argparse.RawTextHelpFormatter,
      description="The changes made will be listed onto <stderr>: \"_\" means \"nothing\".",
      epilog="""Examples:
 > ./utils/outfits/apply_engines.py `find dat/outfits/core_engine/`
 > ./utils/outfits/apply_engines.py -g Krain
""")
   parser.add_argument('-g', '--generate',dest='LINE',help='The generated line name, e.g. Melendez.')
   parser.add_argument('filename', nargs='*', help='An outfit with ".xml" or ".mvx" extension, else will be ignored.\nIf not valid, will not even be printed out.')
   args = parser.parse_args()
   if args.LINE is not None:
      exit(gen_line(args.LINE))
   else:
      exit(main(args.filename))
