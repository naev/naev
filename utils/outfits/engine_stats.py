#!/usr/bin/env python3


from sys import stdout, stderr, stdin
import math

from getconst import PHYSICS_SPEED_DAMP
from combine_multi import mk_combine
from core_outfit import some_outfit
from slst import Slst


out = lambda x: stdout.write(x + '\n')

def line_drawing(gith = False, color = False, term = False):
   head, rule, line, alt_line = Slst([]), Slst([]), Slst([]), Slst([])

   if color or term:
      head += [ ('|', ' ', 1) ]
      [a.append(('', ' |', -1)) for a in [rule, line, alt_line]]
      if color:
         color, alt_col, defc = '\033[30;1m', '\033[34m', '\033[0m'
         rule += [ ('', color, 1), ('', defc, -1) ]
         line += [ ('|', color + '|' + defc) ]
         alt_line += [ ('', alt_col, 1), ('', defc, -1) ]
         #alt_line += [ ('|', (color if term else alt_col) + '|' + defc) ]
      if term:
         rule += [
            ('| -', '|--'), ('- |',  '--|'),
            ('-|-',  "-\N{BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL}-" ),
            ('|-',   "\N{BOX DRAWINGS LIGHT DOWN AND RIGHT}-"           ),
            ('-|',   "-\N{BOX DRAWINGS LIGHT VERTICAL AND LEFT}"        ),
            ('^|',   "\N{BOX DRAWINGS LIGHT VERTICAL AND RIGHT}"        ),
            ('-',    "\N{BOX DRAWINGS LIGHT HORIZONTAL}"                ),
         ]
         line += [ ('|', "\N{BOX DRAWINGS LIGHT VERTICAL}") ]
         alt_line += [
            #('|', "\N{BOX DRAWINGS LIGHT VERTICAL AND RIGHT}", 1),
            #('|', "\N{BOX DRAWINGS LIGHT VERTICAL AND LEFT}", -1),
            #('|', "\N{BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL}")
            ('|', "\N{BOX DRAWINGS LIGHT VERTICAL}")
         ]
      lines = lambda f: alt_line if f else line
   else:
      if gith:
         rule += [ (7*'-', '-'), (3*'-', '-'), (2*'-', '-'), ('-', '---') ]
         line += [ (7*' ', ' '), (3*' ', ' '), (2*' ', ' ') ]
      lines = lambda _: line
   head += line
   if term:
      head = Slst(head[0:1]) + rule + [
         ("\N{BOX DRAWINGS LIGHT VERTICAL AND LEFT}","\N{BOX DRAWINGS LIGHT DOWN AND LEFT}"),
         ("\N{BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL}","\N{BOX DRAWINGS LIGHT DOWN AND HORIZONTAL}")
      ] + line[-1:]
   return head, rule, lines

def field( a, f ):
   try:
      res = a[f]
      if type(res) == type([]):
         res = res[0]

      if type(res) == type(()):
         res = res[0]

      if type(res) == type(''):
         res = float(res)
   except:
      res = 0.0
   return res

accel =        lambda a: field(a, 'accel')
speed =        lambda a: field(a, 'speed')
eml =          lambda a: field(a, 'engine_limit')
turn =         lambda a: field(a, 'turn')

maxspeed =     lambda a: speed(a) + accel(a) / PHYSICS_SPEED_DAMP
fullsptime =   lambda a: maxspeed(a) / accel(a) if accel(a) else 0.0
radius =       lambda a: round(maxspeed(a) / (turn(a)/180.0*math.pi))
fullspdist =   lambda a: round(maxspeed(a) * fullsptime(a) / 2.0)
turntime =     lambda a: 180.0 / turn(a)

def key( A ):
   (a, _) = A
   return (maxspeed(a), accel(a))

def fmt( f ):
   return round(f, 2)

def fmt4( n ):
   s = str(n)
   return (4-len(s))*' ' + ' ' + s + ' '

def l( s ):
   if int(s) == s:
      s = int(s)
   s = str(s)
   if '.' not in s:
      s += '.'
      m = ' '
   else:
      m = '.'
   a, b = tuple(s.split('.', 1))
   return ' | ' + (3-len(a))*' ' + a + m + b + (2-len(b))*' '

def main( args, gith = False, color = False, term = False, autostack = False,
      combine = False, nosort = False, good_only = False ):

   L = [ (o.to_dict(), o.shortname()) for o in mk_combine(args, combine, autostack, good_only) ]

   stderr.write('\n')
   if not nosort:
      L.sort(key = key, reverse = True)

   head, rule, lines = line_drawing(gith, color, term)
   if color:
      C = [ 'eml   \n(t)   ', 'drift \nspeed ', 'max   \nspeed ', 'accel \n      ',
            'fullsp\n(s)   ', 'fullsp\n(km)  ', 'turn  \n(°/s) ', 'turn  \nradius',
             '\xbdturn \n(s)   ']
      if term:
         C = [len(s.split('\n')[0])*'-' + '\n' + s for s in C]
   elif gith:
      C = ['eml (t)', 'drift speed ', 'max speed', 'accel', 'fullsp (s)',
         'fullsp (km)', 'turn (°/s)', 'turn radius', '1/2 turn (s)']
   else:
      C = [' eml  ', 'dr.sp.', 'max sp', 'accel ', 'fsp(s)',
          'fsp.km', ' turn ', 'radius', '1/2turn(s)']

   N = max([3] + [len(n) for (_, n) in L])
   C = [s.split('\n') for s in C]
   while [L for L in C if L!=[]] != []:
      lin = '| '+ N*' ' + ' | ' + ' | '.join([(L+[''])[0] for L in C])
      out(head(lin))
      C = [L[1:] for L in C]

   out(rule('| ---' + (N-3)*'-' + len(C)*' | ------'))
   for count, (k, n) in enumerate(L):
      if accel(k) != 0:
         acc = '| ' + n + (N-len(n))*' ' + ' | '
         acc += fmt4(int(eml(k))) + l(speed(k)) + l(fmt(maxspeed(k)))
         acc += l(accel(k)) + l(fmt(fullsptime(k))) + l(fmt(fullspdist(k)))
         acc += l(turn(k)) + l(radius(k)) + l(fmt(turntime(k)))
         out( lines ((count%4) == 0) (acc) )

if __name__ == '__main__':
   import argparse

   parser = argparse.ArgumentParser(
      formatter_class = argparse.RawTextHelpFormatter,
      description = 'By default, outputs text aligned markdown table comparing the engines respective values, with some new derived fields.',
      epilog = '\nTypical usage (from naev root dir) :\n > find dat/outfits/core_engine/ -name "*.xml" | ./utils/outfits/engines_stats.py'
   )
   parser.add_argument('-g', '--github', action = 'store_true', help = 'unaligned (therefore smaller) valid github md, for use in posts.')
   parser.add_argument('-c', '--color', action = 'store_true', help = 'colored terminal output. You can pipe to "less -RS" if the table is too wide.')
   parser.add_argument('-t', '--term', action = 'store_true', help = 'colored terminal output with extended table characters.')
   parser.add_argument('-A', '--autostack', action = 'store_true', help = 'Outfits are presented both alone and auto-stacked.')
   parser.add_argument('-C', '--combinations', action = 'store_true', help = 'also display all the combinations.' )
   parser.add_argument('-G', '--good', action = 'store_true', help = "Like -C, but good comb. only: don't comb when pri smaller." )
   parser.add_argument('-N', '--no-sort', action = 'store_true', help = "don't sort wrt. max speed" )
   parser.add_argument('-f', '--files', action = 'store_true', help = 'read file list on stdin. Applies when no args.')
   parser.add_argument('filename', nargs = '*', help = """An outfit with ".xml" or ".mvx" extension, else will be ignored.
Can also be two outfits separated by \'+\', or an outfit prefixed with \'2x\' or suffixed with \'x2\'.""")
   args = parser.parse_args()
   if args.term and args.color:
      print('Warning: -c subsumed by -t', file = stderr, flush = True)

   if args.github and (args.color or args.term):
      print('Warning: Ignored conflicting: -g and -' +
         ('t' if args.term else 'c'), file = stderr, flush = True)
      args.github = args.color = args.term = False

   if args.autostack and args.good:
      print('Warning: -A is subsumed by -G', file = stderr, flush = True)
      args.autostack = False
   elif args.autostack and args.combinations:
      print('Warning: -A is subsumed by -C', file = stderr, flush = True)
      args.autostack = False

   if args.good and args.combinations:
      print('Warning: -C is subsumed by -G', file = stderr, flush = True)

   if args.files or args.filename == []:
      args.filename += [l.strip() for l in stdin.readlines()]
   args.filename = [f for f in args.filename if f[-4:] in {".mvx", ".xml"}]
   main(args.filename, args.github, args.color or args.term, args.term,
      args.autostack, args.combinations or args.good, args.no_sort, args.good)
else:
   raise Exception('This module is only intended to be used as main.')
