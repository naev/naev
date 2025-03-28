#!/usr/bin/python

from sys import argv,stderr,stdout
from outfit import outfit
from getconst import PHYSICS_SPEED_DAMP

#TODO: use argparse

AG_EXP  = 0.2
TURN_CT = 0.46

sizes={
   "Za'lek Test Engine":2,
   "Beat Up Small Engine":1,
   "Beat Up Medium Engine":3,
   "Beat Up Large Engine":5,
   'Unicorp Falcon 1400 Engine':4,
   'Tricon Cyclone II Engine':4,
   'Nexus Arrow 700 Engine':3,
   'Melendez Buffalo XL Engine':4,
   'Beat Up Medium Engine':3,
   'Krain Patagium Engine':4,
   'Tricon Cyclone Engine':3,
   'Krain Remige Engine':5,
   'Melendez Mammoth XL Engine':6,
   'Melendez Mammoth Engine':5,
   'Unicorp Eagle 6500 Engine':6,
   'Beat Up Large Engine':5,
   'Tricon Typhoon Engine':5,
   'Tricon Typhoon II Engine':6,
   'Nexus Bolt 3000 Engine':5,
   'Unicorp Hawk 360 Engine':2,
   'Beat Up Small Engine':1,
   'Melendez Ox XL Engine':2,
   "Za'lek Test Engine":2,
   'Tricon Zephyr Engine':1,
   'Tricon Zephyr II Engine':2,
   'Nexus Dart 160 Engine':1
}

def fmt(f):
   res=str(int(round(f)))

   if f>100.0 and res[-1] in "9146" or f>200.0:
      res=str(int(5.0*round(f/5.0)))
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

lines={
   'Nexus'   : 'N',
   'Unicorp' : 'U',
   'Tricon'  : 'T',
   'Krain'   : 'K',
   'Melendez': 'M',
   "Za'lek"  : 'Z',
   "Beat"    : 'B'
}

line_stats = {
    "T" : {
        "ratio" : 2.0, # 2.0 is double accel vs speed (at size 1)
        "speed" : 1.0, # 1.0 indicates current speed rank, lower means slower, higher means faster
        "turn"  : 1.0, # 1.0 means base turn rate, lower is worse, higher is better
    },
    "K" : {
        "ratio" : 1.4, # lower ratio
        "speed" : 1.1, # higher speed than average
        "turn"  : 1.0,
    },
    "N" : {
        "ratio" : 1.0,
        "speed" : 1.0, # Pretty good but slightly slower top speed
        "turn"  : 0.8,
    },
    "M" : {
        "ratio" : 0.5,
        "speed" : 1.0,
        "turn"  : 1.0,
    },
    "U" : {
        "ratio" : 1.0,
        "speed" : 0.8,
        "turn"  : 0.8,
    },
    "Z" : { # TODO make these change over time the profile via Lua
        "ratio" : 1.2,
        "speed" : 0.7,
        "turn"  : 0.8,
    },
    "B" : {
        "ratio" : 0.8,
        "speed" : 0.3,
        "turn"  : 0.6,
    },
}

ALPHA, BETA = 1.11, 0.06

def dec(n):
   if n<=1:
      return 400.0
   else:
      return dec(n-1)/(ALPHA+BETA*(n-1))

def ls2vals(line_size):
   if line_size is None:
      return None
   (line,size) = line_size
   stats = line_stats[line]

   # Modulate full speed based on the speed stat
   fullspeed = dec( size + 1.0 - stats["speed"])

   # Proportion of accel vs speed, defaults so that it is 1:1 at size 1.
   r = 0.5*pow(2,-((size-1)-5)/5) / PHYSICS_SPEED_DAMP

   # Modulate ratio based on outfit
   r *= line_stats[line]["ratio"]

   speed = fullspeed*(1.0-r)
   accel = fullspeed*r*PHYSICS_SPEED_DAMP

   turn = TURN_CT * fullspeed * pow(1.0*accel/speed,AG_EXP) * stats["turn"]
   return {
           "speed":fmt(speed),
           "accel":fmt(accel),
           "turn":fmt_t(turn)
          }

def get_line(o):
   res=o.name().split(' ')[0]
   if res in lines:
      return lines[res]
   else:
      return None

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

if '-h' in argv or '--help' in argv:
   out("Usage: "+argv[0]+" <file1> <file2> ...")
   out("Will only process the files in the list that have .xml or .mvx extension.")
   out("The changes made will be listed onto <stderr>. \"_\" means \"nothing\"")
   out("If an outfit is not recognized as an engine, it won't even be printed out.")
   out("\nTypical usage (from naev root dir) :")
   out("> ./utils/outfits/apply_engines.py `find dat/outfits/core_engine/`")
else:
   outfits = []
   for a in argv[1:]:
      if a.endswith(".xml") or a.endswith(".mvx"):
         o = outfit(a)
         sub = ls2vals(get_line_size(o))
         if sub is not None:
            didit=False
            acc=[]
            for i in o:
               if i.tag in sub and i.text!=sub[i.tag]:
                  acc.append((i.tag,i.text,sub[i.tag]))
                  i.text=sub[i.tag]
                  didit=True
            err(o.fil.split('/')[-1]+': ',nnl=True)
            if didit:
               err(', '.join([i+':'+j+'->'+k for i,j,k in acc]))
               fp=open(o.fil,"w")
               o.write(fp)
               fp.close()
            else:
               err('_')
