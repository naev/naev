#!/usr/bin/env python3

import math
from sys import stderr,stdout
from outfit import outfit,nam2fil,fmtval,unstackvals
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

def fmt(t,half=False):
   red=2 if half and t<45 else 1
   return fmtval(round(red*t)/float(red))

def dec(f):
   n = math.floor(f)
   q = 1.0*f-n
   n = int(n)
   return pow(dec_i(n),1.0-q)*pow(dec_i(n+1),q)

def ls2vals(line,size):
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
      "turn":fmt(turn,True)
   }

def get_line(o):
   res=o.name().split(' ')[0]
   if res in line_stats:
      return res

out=lambda x:stdout.write(x+'\n')
err=lambda x,nnl=False:stderr.write(x+('\n' if not nnl else ''))

def apply_ls(sub,o,additional=dict()):
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

def main(args):
   outfits = []
   for a in args:
      sub=[]
      for doubled in [False,True]:
         o = outfit(a)

         if o is None:
            break

         line = get_line(o)
         if line is None:
            break

         o.autostack(doubled)
         sub.append(ls2vals(line,o.size(doubled)))

      if sub == []:
         continue

      o = outfit(a)

      if o.name() in ["Krain Remige Engine","Za'lek Test Engine"]:
         subs=dict([(k,v1) for k,v1 in sub[0].items()])
      elif o.name()=="Krain Patagium Twin Engine":
         subs=dict([(k,v2) for k,v2 in sub[1].items()])
      else:
         subs=dict([(k,unstackvals(k,v1,sub[1][k])) for k,v1 in sub[0].items()])

      if subs is not None:
         acc=apply_ls(subs,o)
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
