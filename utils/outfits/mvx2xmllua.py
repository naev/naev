#!/usr/bin/env python3

keep_in_xml = set(['priority', 'rarity'])

from sys import argv, stderr, stdin, stdout, exit
from outfit import outfit, nam2fil, MOBILITY_PARAMS, text2val, roundit, ET


def fmt( f ):
   return str(roundit(f))

def sfmt( f ):
   res = fmt(f)
   if res == '0':
      return '_'
   elif res[0] != '-':
      return '+'+res
   else:
      return res;

def _process_group( r, field ):
   acc = []
   r = r.find(field)
   torem = []
   for e in r.iter():
      t = e.tag
      try:
         a, b = text2val(e.text)
      except:
         continue

      if t == 'price':
         e.text = fmt(round((a+b)/2,-2))
      elif t in keep_in_xml and a == b:
         continue
      else:
         if a == b:
            e.text = fmt(a)
         acc.append((t,(a, b)))
         torem.append((r, e))

   return acc, torem

def _mklua( L ):
   ind = 3*' '
   output = '\nrequire("outfits.lib.multicore").init{\n'

   for (nam,(main, sec)) in L:
      if nam not in keep_in_xml:
         output += ind+'{ "'+nam+'", '
         output += fmt(main)
         if main != sec:
            output += ', '+fmt(sec)
         output += '},\n'

   return output+'}\n'

def _toxmllua( o ):
   R = o.r
   acc1, tr1 = _process_group(R, './general')
   acc2, tr2 = _process_group(R, './specific')

   found = False
   for e in R.findall('./specific'):
      for elt in e:
         if elt.tag == 'lua_inline':
            found = True
         break
      break

   for (r, e) in tr1+tr2:
      r.remove(e)

   el = None
   for e in R.findall('./specific'):
      for elt in e:
         if elt.tag == 'lua_inline':
            el = elt
            break
      if el is None:
         el = ET.Element('lua_inline')
         el.text = ''
         e.append(el)
      el.text = _mklua(acc1+acc2) + el.text
      break

def mvx2xmllua( argin, argout, quiet ):
   o = outfit(argin)
   if o is not None:
      nam = nam2fil(o.name())
      _toxmllua(o)
      if not quiet:
         stderr.write('mvx2xmllua: ' + (nam if argout == '-' else argout) + '\n')
      o.write(argout)
   return o

if __name__ == '__main__':
   import argparse

   parser = argparse.ArgumentParser(
      description =
   """Takes an extended outfit as input and outputs a xml (potentially with inlined lua) on output.
The name the output should have is written on <stderr>.
If the input is invalid, nothing is written on stdout and stderr and non-zero is returned.
The special values "-" mean stdin/stdout.
"""
   )
   parser.add_argument('input', nargs = '?', default = "-")
   parser.add_argument('output', nargs = '?', default = "-")
   parser.add_argument('-q', '--quiet', action = 'store_true')
   args = parser.parse_args()
   o = mvx2xmllua(args.input, args.output, args.quiet)
   exit(1 if o is None else 0)
