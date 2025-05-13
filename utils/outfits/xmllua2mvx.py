#!/usr/bin/env python3

# Tries to revert xml with inlined lua using multicore back to multi-valued xml.
# exits 1 if anything turns wrong.


import re
from outfit import outfit, ET
from sys import argv, stderr, exit


# everything ont in this list goes to specific intead of general
general = ['mass', 'cpu']

def numeval(s):
   try:
      return int(s)
   except:
      return float(s)

def parse_lua_multicore( si ):
   name = ' ("|\')([^"\']*)\\1'
   sep = ' ,'
   num = ' -? [0-9]+(\\.[0-9]+)?'

   expr = ' require \\(? ("|\')outfits.lib.multicore(\\1) \\)? \\. init \\{ '
   block = ' \\{ ((' + name + sep + num + ' (' + sep + num + ')?) (' + sep + ')?'+ ' ) \\}'
   expr = expr + ' ((' + block + ' ) ( ,' + block + ' )* ,? ) \\} '
   expr = expr.replace(' ', '\\s*')

   block = ' \\{ ("|\')(?P<name>[^"\']*)\\1'+sep+' (?P<pri>'+num+') ('+sep+' (?P<sec>'+num+'))? (' + sep + ' )? \\}'
   block = block.replace(' ', '\\s*')

   s = re.sub('\n', ' ', si)
   match = re.search(expr, s)
   if match is None:
      return [], si

   L = [t.groupdict() for t in re.finditer(block, match.group(3))]
   for d in L:
      if d['sec'] is None:
         d['sec'] = d['pri']
   L = [(d['name'], numeval(d['pri']), numeval(d['sec'])) for d in L]
   return L, si[match.span()[1]:]

def xmllua2mvx( argin, argout, quiet = False ):
   try:
      o = outfit(argin)
      fields, li = parse_lua_multicore(o.find('lua_inline'))
   except:
      return None

   if not quiet:
      stderr.write('xmllua2mvx: ' + o.name() + '\n')

   d = {'general': [], 'specific': []}
   for t in fields:
      if t[0] in general:
         d['general'].append(t)
      else:
         d['specific'].append(t)

   for e in o:
      if e.tag in ['general', 'specific']:
         for (a, b, c) in d[e.tag]:
            el = ET.Element(a)
            el.text = str(b)
            if b != c:
               el.text += '/' + str(c)
            e.append(el)
         if e.tag == 'specific':
            spe = e
      elif e.tag == 'lua_inline':
         if li.strip() == '':
            spe.remove(e)
         else:
            e.text = li.strip()
   o.write(argout)
   return o

if __name__ == '__main__':
   import argparse

   parser = argparse.ArgumentParser(
      description =
   """Takes a xml outfit as input (potentially with inlined lua) and produces a mvx on output.
The name the output should have is written on <stderr>.
If the input is invalid, nothing is written on stdout and stderr and non-zero is returned.
The special values "-" mean stdin/stdout.
"""
   )
   parser.add_argument('-q', '--quiet', action = 'store_true')
   parser.add_argument('input', nargs = '?', default = '-')
   parser.add_argument('output', nargs = '?', default = '-')
   args = parser.parse_args()
   o = xmllua2mvx(args.input, args.output, args.quiet)
   exit(1 if o is None else 0)
