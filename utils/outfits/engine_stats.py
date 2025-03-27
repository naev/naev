#!/usr/bin/python

from outfit import outfit

from sys import argv
import math


def field(a,f):
   res=0.0
   try:
      res=a[f]
      if type(res)==type([]):
         res=res[0]

      if type(res)==type((1.0,)):
         res=res[0]
         
      if type(res)==type(1.0):
         return res
   except:
      pass
   return res

accel=lambda a:field(a,'accel')
speed=lambda a:field(a,'speed')
turn=lambda a:field(a,'turn')
 
maxspeed=lambda a:speed(a)+accel(a)/3.0
fullsptime=lambda a:maxspeed(a)/accel(a) if accel(a) else 0.0
radius=lambda a:round(maxspeed(a)/(turn(a)/180.0*math.pi))
fullspdist=lambda a:round(maxspeed(a)*fullsptime(a)/2.0)

def my_cmp((a,_),(b,_2)):
   d=maxspeed(a)-maxspeed(b)
   if d==0.0:
      d=accel(a)-accel(b)
   return -(-1 if d<0.0 else 1 if d>0.0 else 0)

def fmt(f):
   return round(f,2)

def l(s):
   if int(s)==s:
      s=int(s)
   s=str(s)
   if '.' not in s:
      s+='.'
      m=' '
   else:
      m='.'
   a,b=tuple(s.split('.',1))
   return '|'+(3-len(a))*' '+a+m+b+(2-len(b))*' '

L=[(a.to_dict(),a.shortname()) for a in map(outfit,argv[1:]) if a is not None]
L.sort(my_cmp)
C=['speed','max speed','accel','fullsp time','fullsp dist','turn','turn radius']
N=max([len(n) for (_,n) in L])
print '|'+'  '+'|',' | '.join(C)
print '|  ---'+(N-3)*' '+len(C)*'|  ---  '
for k,n in L:
   if accel(k)!=0:
      nam=n+(N-len(n))*' '
      print '|',nam,l(speed(k)),l(fmt(maxspeed(k))),l(accel(k)),
      print l(fmt(fullsptime(k))),l(fmt(fullspdist(k))),
      print l(turn(k)),l(radius(k))


