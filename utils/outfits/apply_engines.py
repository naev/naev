#!/usr/bin/python

from sys import argv,stderr,stdout
from outfit import outfit
from getconst import PHYSICS_SPEED_DAMP

#TODO: use argparse

AG_EXP=0.2
TURN_CT=0.46

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
   'Nexus':'N',
   'Unicorp':'N',
   'Tricon':'T',
   'Krain':'K',
   'Melendez':'M',
   "Za'lek":'Z',
   "Beat":'B'
}

alpha,beta=1.11,0.06

def dec(n):
   if n<=1:
      return 400.0
   else:
      return dec(n-1)/(alpha+beta*(n-1))

def ls2vals(line_size):
   if line_size is None:
      return None

   (line,size)=line_size
   fullspeed=dec(size)

   if line in ['N','M','Z','B']:
      fullspeed*=7.0/8.0

   r=0.15*pow(2,-((size-1)-2.5)/2.5)

   if line in ['T','K']:
      r*=1.75
   elif line=='M':
      r*=0.7

   speed,acc=fullspeed*(1.0-r),fullspeed*r*PHYSICS_SPEED_DAMP

   if line=='K':
      speed*=1.05
      acc*=1.3
   elif line=='Z':
      speed*=0.7
      acc*=0.7
   elif line=='B':
      speed*=0.55
      acc*=0.55

   fullspeed=speed+acc/PHYSICS_SPEED_DAMP

   turn=TURN_CT*fullspeed*pow((acc/PHYSICS_SPEED_DAMP)/speed,AG_EXP)
   return {"speed":fmt(speed),"accel":fmt(acc),"turn":fmt_t(turn)}

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
   for a in argv[1:]:
      if a.endswith(".xml") or a.endswith(".mvx"):
         o=outfit(a)
         sub=ls2vals(get_line_size(o))
         if sub is not None:
            didit=False
            acc=[]
            for i in o:
               if i.tag in sub and i.text!=sub[i.tag]:
                  acc.append((i.tag,i.text,sub[i.tag]))
                  i.text=sub[i.tag]
                  didit=True
            err(a.split('/')[-1]+': ',nnl=True)
            if didit:
               err(', '.join([i+':'+j+'->'+k for i,j,k in acc]))
               fp=open(a,"w")
               o.write(fp)
               fp.close()
            else:
               err('_')
