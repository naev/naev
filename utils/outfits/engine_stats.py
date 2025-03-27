#!/usr/bin/env python

from outfit import outfit

from sys import argv,stdout
import math

out=lambda x:stdout.write(x+'\n')

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

def key(A):
   (a,_)=A
   return (maxspeed(a),accel(a))

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
   return ' | '+(3-len(a))*' '+a+m+b+(2-len(b))*' '

if '-h' in argv or '--help' in argv:
   out("Usage: "+argv[0]+" <file1> <file2> ...")
   out("Will only process the files in the list that have .xml or .mvx extension.")
   out("Outputs a MD table with some new derived fields.")
   out("\nTypical usage (from naev root dir) :")
   out("> ./utils/outfits/apply_engines.py `find dat/outfits/core_engine/`")
else:
   L=[(a.to_dict(),a.shortname()) for a in map(outfit,argv[1:]) if a is not None]
   L.sort(key=key,reverse=True)
   C=['speed','max speed','accel','fullsp time','fullsp dist','turn','turn radius']
   N=max([len(n) for (_,n) in L])
   out('| | '+' | '.join(C))
   out('| ---'+(N-2)*' '+len(C)*'| ---    ')
   for k,n in L:
      if accel(k)!=0:
         nam=n+(N-len(n))*' '
         acc='| '+nam+l(speed(k))+l(fmt(maxspeed(k)))+l(accel(k))
         acc+=l(fmt(fullsptime(k)))+l(fmt(fullspdist(k)))+l(turn(k))+l(radius(k))
         out(acc)


