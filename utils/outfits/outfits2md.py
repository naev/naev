#!/usr/bin/env python3

from sys import stderr, stdin
from outfit import LOWER_BETTER
from arg_to_obj import arg_to_naev_obj

def transpose( M ):
   N = max(map(len, M))
   M = [t + ['']*(N-len(t)) for t in M]
   return list(zip(*(tuple(M))))

getfloat = lambda s: float(s.split('/')[0])

def keyfunc( s ):
   def key( o ):
      try:
         return getfloat(o[s])
      except:
         return None
   return key

def nfmt( n ):
   f = float(n)
   if f == round(f):
      f = int(f)
   return str(f)

def main( args, gith = False, color = False, term = False, noext = False,
      sortit = False, autostack = False, combine = False, good = False ):
   if not args:
      return 0

   rang = dict()
   L = []
   names = []
   acc = []

   for o in arg_to_naev_obj(args, combine, autostack, good):
      d = o.to_dict()
      for k, v in d.items():
         if type(v) == type(()):
            (val, _) = v
            s = nfmt(v[0]) + '/' + nfmt(v[1])
         elif type(v) in {type(1), type(1.0)}:
            val = v
            s = nfmt(v)
         else:
            continue

         d[k] = s
         if k not in rang:
            rang[k] = (val, val)
            acc.append(k)
         else:
            (mi, ma) = rang[k]
            rang[k] = (min(mi, val), max(ma, val))

      names.append(o.shortname())
      L.append(d)
   stderr.write('\n')

   for i, t in rang.items():
      (m, M) = t
      if i in LOWER_BETTER:
         rang[i] = (M, m)
      if noext:
         rang[i] = (None, None)

   if sortit:
      for i, o in enumerate(L):
         o['name'] = names[i]
      L.sort(key = keyfunc(sortit))
      for i, o in enumerate(L):
         names[i] = o['name']

   Res = [[k]+[l[k] if k in l else '' for l in L] for k in acc]
   names = [''] + names
   length = [4] * len(names) # :--- at least 3 - required

   termit_rule = lambda s: s
   if color:
      Sep, SepAlt, Lm, Rm, LM, RM = '\033[30;1m|\033[0m', '\033[34m|\033[0m', '\033[31m', '\033[37m', '\033[32m', '\033[37m'
      mk_pad = lambda i, n: '\033[34m' + n*'-' + '\033[0m'
      leng = lambda x: len(x) - (10 if x != '' and x[0]=='\033' else 0)
      if term:
         # Could also use it to manage gith and color.
         from slst import Slst
         termit = Slst([('|',"\N{BOX DRAWINGS LIGHT VERTICAL}")])
         (Sep, SepAlt, Lm, Rm, LM, RM) = map(termit, (Sep, SepAlt, Lm, Rm, LM, RM))
         termit_rule = -termit + [
            ('\033[34m', ), ('\033[0m', ),            # uncolor
            ('| -', '|--'), ('- |',  '--|'),
            ('-|-',  "-\N{BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL}-" ),
            ('|-',   "\N{BOX DRAWINGS LIGHT DOWN AND RIGHT}-"           ),
            ('-|',   "-\N{BOX DRAWINGS LIGHT VERTICAL AND LEFT}"        ),
            ('^|',   "\N{BOX DRAWINGS LIGHT VERTICAL AND RIGHT}"        ),
            ('-',    "\N{BOX DRAWINGS LIGHT HORIZONTAL}"                ),
            ('', '\033[34m', 1), ('', '\033[0m', -1)  # recolor whole line
         ]
   else:
      Sep, SepAlt, Lm, Rm, LM, RM = '|', '|', '_', '_', '**', '**'
      mk_pad = lambda i, n: '-'*(n-1)+('-' if i == 0 else ':')
      leng = len

   if color:
      head = transpose([s.split(' ') for s in names])
   else:
      head = [names]

   if not gith:
      for r in Res + head:
         length = [max(n, len(s)) for (n, s) in zip(length, r)]

   mklinsep = lambda sep: lambda L:sep+' '+(' '+sep+' ').join(L)+' '+sep
   mklinalt = lambda alt: mklinsep(SepAlt) if alt else mklinsep(Sep)
   mklin = mklinalt(False)
   fmt = lambda t: (t[1]-leng(t[0]))*' '+t[0]
   lfmt = lambda t: t[0]+(t[1]-leng(t[0]))*' '

   def emph( s, m ):
      (mi, ma) = m
      if s == '':
         return '_'
      elif mi != ma:
         if getfloat(s) == mi:
            return Lm + s + Rm
         elif getfloat(s) == ma:
            return LM + s + RM
      return s

   print()
   if term:
      toprule = (length[0]+3)*' ' + mklinalt(True)([mk_pad(i, n) for i, n in enumerate(length[1:])])
      print((termit_rule + [ ('\033[34m', '\033[30;1m'),
         ("\N{BOX DRAWINGS LIGHT VERTICAL AND LEFT}","\N{BOX DRAWINGS LIGHT DOWN AND LEFT}"),
         ("\N{BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL}","\N{BOX DRAWINGS LIGHT DOWN AND HORIZONTAL}")
      ])(toprule))

   for t in head:
      acc = mklin(map(fmt, zip(t, length)))
      if color:
         acc = ' '+acc[len(Sep):]
      print(acc)
   print(termit_rule(mklinalt(True)([mk_pad(i, n) for i, n in enumerate(length)])))

   count = 0
   for r in Res:
      r = [r[0].replace('_', ' ')] + [emph(k, rang[r[0]]) for k in r[1:]]
      print(mklinalt(count%3 == 2)([fmt(x) if i>0 else lfmt(x) for i, x in enumerate(zip(r, length))]))
      count += 1
   return 0

if __name__ == '__main__':
   import argparse

   parser = argparse.ArgumentParser(
      usage = ' %(prog)s  [-g|-c] [-n] [(-s SORT) | -S]  [filename ...]',
      formatter_class = argparse.RawTextHelpFormatter,
      description = 'By default, outputs text aligned markdown table comparing the outfits respective values.',
      epilog = '\nTypical usage (from naev root dir) :\n > ./utils/outfits/outfits2md.py dat/outfits/core_system/small/*.xml -C -t | less -RS'
   )
   parser.add_argument('-g', '--github', action = 'store_true', help = 'unaligned (therefore smaller) valid github md, for use in posts.')
   parser.add_argument('-c', '--color', action = 'store_true', help = 'colored terminal output. You can pipe to "less -RS" if it is too wide.')
   parser.add_argument('-t', '--term', action = 'store_true', help = 'colored terminal output (TODO: with extended table characters).')
   parser.add_argument('-n', '--nomax', action = 'store_true', help = 'Do not emphasize min/max values.' )
   parser.add_argument('-s', '--sort', help = 'inputs are sorted by their SORT key.')
   parser.add_argument('-S', '--sortbymass', action = 'store_true', help = 'Like -s mass.' )
   parser.add_argument('-A', '--autostack', action = 'store_true', help = 'Outfits are presented both alone and auto-stacked.')
   parser.add_argument('-C', '--combinations', action = 'store_true', help = 'Does all the combinations.' )
   parser.add_argument('-G', '--good', action = 'store_true', help = "Like -C, but good comb. only: don't comb when pri smaller." )
   parser.add_argument('-f', '--files', action = 'store_true', help = 'read file list on stdin. Applies when no args.')
   parser.add_argument('filename', nargs = '*', help = """An outfit with ".xml" or ".mvx" extension, else will be ignored.
Can also be two outfits separated by \'+\', or an outfit prefixed
with \'2x\' (or: \'1x\') or suffixed with \'x2\' (or: \'x1\').""")

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

   ign = [f for f in args.filename if not f.endswith('.xml') and not f.endswith('.mvx')]
   if ign:
      print('Ignored: "'+'", "'.join(ign)+'"', file = stderr, flush = True)

   sortby = args.sort or (args.sortbymass and 'mass') or False

   if sortby:
      print('sorted by "'+str(sortby)+'"', file = stderr, flush = True)

   if args.files or not args.filename:
      args.filename += [l.strip() for l in stdin.readlines()]

   main([f for f in args.filename if f not in ign],
      args.github, args.color or args.term, args.term, args.nomax, sortby,
      args.autostack, args.combinations or args.good, args.good)
else:
   raise Exception('This module is only intended to be used as main.')
