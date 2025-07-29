# python3

# Tries to revert xml with inlined lua using multicore back to multi-valued xml.
# exits 1 if anything turns wrong.


import re
from outfit import outfit, ET
from sys import argv, stderr, exit


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

def xmllua2mvx( argin, argout, quiet = False, multicore_only = False ):
   try:
      o = outfit(argin)
      t = o.find('lua_inline')
      fields, li = parse_lua_multicore(t or '')
   except:
      return None

   o.is_multi = (fields != [])
   d = {'general': [], 'specific': fields}

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
   if o.is_multi or not multicore_only:
      if not quiet:
         stderr.write('xmllua2mvx: ' + o.name() + '\n')
      o.write(argout)
   return o
