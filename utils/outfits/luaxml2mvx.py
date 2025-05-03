#!/usr/bin/env python3

# Tries to revert xml with inlined lua using multicore back to multi-valued xml.
# exits 1 if anything turns wrong.


import re
from outfit import outfit,ET
from sys import argv,exit

general = ['mass']


def fmt(n):
   f = float(n)
   if round(f) == f:
      return int(f)
   else:
      return f

def parse_lua_multicore(s):
   s = re.sub('\s','',s)
   expr = re.escape('require(\"outfits.lib.multicore\").init{')
   block = "\\{([^{}]*)\\}"
   expr = expr+'(('+block+')(,'+block+')*,?'+')\\}'
   match = re.search(expr,s)

   for t in re.finditer(block, match.group(1)):
      L = str(t.group(1)).split(',')
      if L[-1] == '':
         L = L[:-2]
      (a,b,c) = tuple(L)
      yield (eval(a),fmt(b),fmt(c))

def do_it():
   o = outfit(argv[1])
   d={'general':[],'specific':[]}
   for t in parse_lua_multicore(o.to_dict()['lua_inline']):
      if t[0] in general:
         d['general'].append(t)
      else:
         d['specific'].append(t)

   for e in o:
      if e.tag in ['general','specific']:
         for (a,b,c) in d[e.tag]:
            el=ET.Element(a)
            if b==c:
               el.text=str(b)
            else:
               el.text=str(b)+'/'+str(c)
            e.append(el)
         if e.tag == 'specific':
            spe=e
      elif e.tag == 'lua_inline':
         spe.remove(e)
   o.write("-")

if __name__=="__main__":
   res=0
   try:
      do_it()
   except:
      res=1

   exit(res)
