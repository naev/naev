#!/usr/bin/env python3

from getconst import PHYSICS_SPEED_DAMP

from outfit import outfit

from sys import stdout
import math

out = lambda x:stdout.write(x+'\n')

def field(a,f):
   res = 0.0
   try:
      res = a[f]
      if type(res) == type([]):
         res = res[0]

      if type(res) == type((1.0,)):
         res = res[0]

      if type(res) == type(1.0):
         return res
   except:
      pass
   return res

accel = lambda a:field(a,'accel')
speed = lambda a:field(a,'speed')
eml = lambda a:field(a,'engine_limit')
turn = lambda a:field(a,'turn')

maxspeed = lambda a:speed(a)+accel(a)/PHYSICS_SPEED_DAMP
fullsptime = lambda a:maxspeed(a)/accel(a) if accel(a) else 0.0
radius = lambda a:round(maxspeed(a)/(turn(a)/180.0*math.pi))
fullspdist = lambda a:round(maxspeed(a)*fullsptime(a)/2.0)

turntime = lambda a:180.0/turn(a)

def key(A):
   (a,_) = A
   return (maxspeed(a),accel(a))

def fmt(f):
   return round(f,2)

def fmt4(n):
   s = str(n)
   return (4-len(s))*' '+' '+s+' '

def l(s):
   if int(s) == s:
      s = int(s)
   s = str(s)
   if '.' not in s:
      s += '.'
      m = ' '
   else:
      m = '.'
   a,b = tuple(s.split('.',1))
   return ' | '+(3-len(a))*' '+a+m+b+(2-len(b))*' '

def main(args, gith = False, color = False, autostack=False, combine=False, nosort=False, filt=False):
   sec_args = []
   if combine or autostack:
      for i in args:
         if i[:2] == '2x' or i[-2:] == 'x2':
            stderr.write('"2x"')
         elif '+' in i:
            stderr.write('"+"')
         else:
            if outfit(i).can_sec():
               sec_args.append(i)
            continue
         stderr.write(' incompatible with -A/-C options\n')
         return 1

      if combine:
         sec_args = set(sec_args)
         tmp = [t for t in args]
         for i in range(len(args)):
            for j in range(i, len(args)):
               if args[j] in sec_args:
                  tmp.append(args[i]+'+'+args[j])
               if i != j and args[i] in sec_args:
                  tmp.append(args[j]+'+'+args[i])
         args = tmp
      elif autostack:
         args = args+['2x'+i for i in sec_args]

   L = []
   for i in range(len(args)):
      if args[i][:2] == '2x' or args[i][-2:] == 'x2':
         args[i] = args[i][2:] if args[i][:2] == '2x' else args[i][:-2]
         args[i] = args[i]+'+'+args[i]

      if len(args[i].split('+')) == 2:
         o,o2 = args[i].split('+')
         o,o2 = outfit(o.strip()),outfit(o2.strip())
         if not o.can_stack(o2):
            continue
         if filt and o.size() < o2.size():
            continue
         o.stack(o2)
      else:
         o = outfit(args[i])
         if not o.can_alone():
            continue
      L.append(o)

   L = [(o.to_dict(),o.shortname()) for o in L]
   if not nosort:
      L.sort(key = key,reverse = True)

   if color:
      altgrey = '\033[34m'
      grey = '\033[30;1m'
      greyit = lambda s:s.replace('|',grey+'|\033[0m')
      C = ['eml   \n(t)   ','drift \nspeed ','max   \nspeed ','accel \n      ','fullsp\n(s)   ','fullsp\n(km)  ','turn  \n(°/s) ','turn  \nradius','1/2turn\n(s)    ']
   elif gith:
      C = ['eml (t)','drift speed ','max speed','accel','fullsp (s)','fullsp (km)','turn (°/s)','turn radius','1/2turn (s)']
   else:
      C = [' eml  ','dr.sp.','max sp','accel ','fsp(s)','fsp.km',' turn ','radius','1/2 turn (s)']
   N = max([0]+[len(n) for (_,n) in L]) if not gith else 0
   if color:
      C = [tuple(s.split('\n')) for s in C]
      lin = '  '+(N)*' '+' | '+' | '.join([a for (a,b) in C])
      out(greyit(lin))
      C = [b for (a,b) in C]
   acc = '| '+(N)*' '+' | '+' | '.join(C)
   if color:
      acc = ' '+acc[1:]
      acc = greyit(acc)
   out(acc)
   lin = '| ---'+(N-3)*'-'+' '+len(C)*('| ---'+('---' if not gith else '')+' ')
   if color:
      lin = altgrey+lin+"\033[0m"
      count = 0
   out(lin)
   for k,n in L:
      if accel(k) != 0:
         nam = n + ((N-len(n))*' ' if not gith else '')
         acc = '| '+nam+' | '+fmt4(int(eml(k)))
         acc += l(speed(k))+l(fmt(maxspeed(k)))+l(accel(k))
         acc += l(fmt(fullsptime(k)))+l(fmt(fullspdist(k)))+l(turn(k))+l(radius(k))
         acc += l(fmt(turntime(k)))
         if gith:
            acc = acc.replace('  ',' ').replace('  ',' ')
         elif color:
            acc = greyit(acc)
            if (count%3) == 2:
               acc = acc.replace(grey,altgrey)
            count += 1
         out(acc)

if __name__ == '__main__':
   import argparse

   parser = argparse.ArgumentParser(
      formatter_class = argparse.RawTextHelpFormatter,
      description = "By default, outputs text aligned markdown table comparing the engines respective values, with some new derived fields.",
      epilog = "\n Typical usage (from naev root dir) :\n > ./utils/outfits/apply_engines.py `find dat/outfits/core_engine/`"
   )
   parser.add_argument('-g', '--github', action = 'store_true', help = "unaligned (therefore smaller) valid github md, for use in posts.")
   parser.add_argument('-c', '--color', action = 'store_true', help = "colored terminal output. You can pipe to \"less -RS\" if the table is too wide.")
   parser.add_argument('-A', '--autostack', action = 'store_true', help = "also display x2 outfits")
   parser.add_argument('-C', '--combinations', action = 'store_true', help = "also display all the combinations." )
   parser.add_argument('-N', '--no-sort', action = 'store_true', help = "don't sort wrt. max speed" )
   parser.add_argument('-f', '--filter', action = 'store_true', help = "don't do comb where pri is smaller" )
   parser.add_argument('filename', nargs = '*', help = """An outfit with ".xml" or ".mvx" extension, else will be ignored.
Can also be two outfits separated by \'+\', or an outfit prefixed with \'2x\' or suffixed with \'x2\'.""")
   args = parser.parse_args()
   if args.autostack and args.combinations:
      print("Warning: -A subsumed by -C", file = stderr, flush = True)
      args.autostack = False

   main(args.filename, args.github, args.color, args.autostack, args.combinations, args.no_sort, args.filter)
else:
   raise Exception("This module is only intended to be used as main.")
