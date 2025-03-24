#!/usr/bin/python

from sys import argv,stderr
from outfit import outfit

sizes={
   "Za'lek Test Engine":2,
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
   "Za'lek":'Z'
}

alpha,beta=1.0,0.1

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

   if line in ['N','M','Z']:
      fullspeed*=7.0/8.0

   r=0.15*pow(2,-((size-1)-2.5)/2.5)

   if line in ['T','K']:
      r*=2
   elif line=='M':
      r*=0.7

   speed,acc=fullspeed*(1.0-r),fullspeed*r*3.0

   if line=='K':
      speed*=1.05
      acc*=1.3
   elif line=='Z':
      speed*=0.7
      acc*=0.7

   turn=speed/5.0+acc/4.0
   return {"speed":fmt(speed),"accel":fmt(acc),"turn":fmt_t(turn)}

def get_line(o):
   res=o.name().split(' ')[0]
   if lines.has_key(res):
      return lines[res]
   else:
      return None

def get_size(o):
   res=o.name()
   if sizes.has_key(res):
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

for a in argv[1:]:
   if a.endswith(".xml") or a.endswith("mvx"):
      o=outfit(a)
      sub=ls2vals(get_line_size(o))
      if sub is not None:
         didit=False
         acc=[]
         for i in o:
            if sub.has_key(i.tag) and i.text!=sub[i.tag]:
               acc.append((i.tag,i.text,sub[i.tag]))
               i.text=sub[i.tag]
               didit=True
         print >>stderr,a.split('/')[-1]+':',
         if didit:
            print >>stderr,', '.join([i+':'+j+'->'+k for i,j,k in acc])
            fp=file(a,"w")
            o.write(fp)
            fp.close()
         else:
            print >>stderr,'_'
