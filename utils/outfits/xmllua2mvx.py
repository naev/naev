#!/usr/bin/env python3

# Tries to revert xml with inlined lua using multicore back to multi-valued xml.
# exits 1 if anything turns wrong.


import re
from outfit import outfit, ET
from sys import argv, exit


# everything ont in this list goes to specific intead of general
general = ['mass','cpu']


def parse_lua_multicore(si):
   s = re.sub('\n', ' ', si)

   name = ' ("|\')([^"\']*)\\1'
   sep = ' ,'
   num = ' -? [0-9]+(\.[0-9]+)?'

   expr = ' require \( ("|\')outfits.lib.multicore(\\1) \) \. init \{ '
   block = ' \\{ ((' + name + sep + num + sep + num + ') (' + sep + ')?'+ ' ) \\}'
   expr = expr + ' ((' + block + ' ) ( ,' + block + ' )* ,? ) \\} '
   expr = expr.replace(' ', '\s*')
   match = re.search(expr, s)
   if match is None:
      return [],si

   block = ' \\{ ("|\')(?P<name>[^"\']*)\\1'+sep+' (?P<pri>'+num+')'+sep+' (?P<sec>'+num+') (' + sep + ' )? \\}'
   block = block.replace(' ','\s*')
   L = [t.groupdict() for t in re.finditer(block, match.group(3))]
   L = [(d['name'], eval(d['pri']), eval(d['sec'])) for d in L]
   return L, si[match.span()[1]:]

def do_it():
   o = outfit(argv[1])
   d={'general':[], 'specific':[]}
   fields, li = parse_lua_multicore(o.to_dict()['lua_inline'])

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
            if b!= c:
               el.text = el.text + '/' + str(c)
            e.append(el)
         if e.tag == 'specific':
            spe = e
      elif e.tag == 'lua_inline':
         if li.strip() == '':
            spe.remove(e)
         else:
            e.text = li.strip()
   o.write("-")

if __name__=="__main__":
   res = 0
   do_it()

   """
   try:
      do_it()
   except:
      res=1
   """

   exit(res)
