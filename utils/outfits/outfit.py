#!/usr/bin/env python3


import sys
#from sys import stderr
from os import path
script_dir = path.join(path.dirname(__file__), '..')
sys.path.append(path.realpath(script_dir))
from xml_name import xml_name as nam2fil
from naev_xml import naev_xml
import re

MOBILITY_PARAMS = {'speed', 'turn', 'accel', 'thrust'}
KEEP_IN_XML = {'priority', 'rarity', 'price'}
LOWER_BETTER = {'mass', 'price', 'delay', 'ew_range', 'falloff', 'trackmin', 'trackmax', 'dispersion', 'speed_dispersion', 'energy_regen_malus', 'ew_stealth', 'ew_stealth_timer', 'ew_signature', 'launch_lockon', 'launch_calibration', 'fwd_energy', 'tur_energy', 'ew_track', 'cooldown_time', 'cargo_inertia', 'land_delay', 'jump_delay', 'delay', 'reload_time', 'iflockon', 'jump_warmup', 'rumble', 'ammo_mass', 'time_mod', 'ew_hide', 'launch_reload'}

def shorten( s ):
   L = s.split(' ')
   while L and L[0][1:2] == '.':
      L = L[1:]

   if not L:
      return '???'
   elif L[0] == 'Beat':
      if L[2] == 'Medium':
         L[2] = 'Med.'
      return 'B. ' + L[2]
   else:
      return L[0]

def prisec( tag, r1, r2, eml1, eml2 ):
   if tag in MOBILITY_PARAMS:
      return round(2.0 * float(r1*eml1 + r2*eml2) / float(eml1 + eml2)) / 2
   else:
      return r1 + r2

def parse_lua_multicore( si ):
   name = ' ("|\')([^"\']*)\\1'
   sep = ' ,'
   num = ' -? [0-9]+(\\.[0-9]+)?'

   expr = ' require \\(? ("|\')outfits.lib.multicore(\\1) \\)? \\. init \\( \\{ '
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
   L = [(d['name'], d['pri'], d['sec']) for d in L]
   return L, si[:match.span()[0]], si[match.span()[1]:]

def un_multicore( o ):
   try:
      e = o.find('lua_inline', ref = True)
      L, bef, aft = parse_lua_multicore( e['lua_inline'] )
   except:
      return False

   paren, args, crt = 1, [], ''
   for i, c in enumerate(aft):
      if c == ',' and paren == 1:
         args.append(crt.strip())
         crt = ''
      else:
         if c == '(':
            paren += 1
         elif c == ')':
            paren -= 1
            if paren == 0:
               break
         crt += c
   else:
      return False

   if crt:
      args.append(crt.strip())
   aft = aft[i+1:].strip()
   bef = bef.strip()

   e['lua_inline'] = bef
   if args:
      e['multicore_args'] = args
   if aft:
      e['lua_inline_post'] = aft

   if not (dst := o.find('specific')):
      dst = o['specific'] = {}
   for (k, v1, v2) in L:
      if v1 == v2:
         dst[k] = v1
      else:
         dst[k] = {'pri': v1, 'sec': v2}
   return True

class outfit(naev_xml):
   # None means auto
   def __init__( self, filename, is_multi = None, read_only = False ):
      self.pri = None
      naev_xml.__init__(self, filename, read_only = read_only)
      if 'outfit' not in self:
         raise Exception('Invalid outfit filename "' + repr(filename) + '"')
      self.short = None
      self.is_multi = False
      if is_multi or is_multi is None:
         if un_multicore(self):
            self.is_multi = True
            self._uptodate = True
         elif is_multi:
            raise ValueError('"' + filename +'" is not a valid multicore.')

   def can_pri_sec( self ):
      if self.pri is None:
         k = self.find('slot')
         self.pri = '@prop' in k and k['@prop'].find('secondary') == -1
         self.sec = '@prop_extra' in k and k['@prop_extra'].find('secondary') != -1
         self.sec |= '@prop' in k and k['@prop'].find('secondary') != -1
      return self.pri, self.sec

   can_pri = lambda self: self.can_pri_sec()[0]
   can_sec = lambda self: self.can_pri_sec()[1]

   def can_alone( self ):
      return self.name().find('Twin') == -1

   def can_stack( self, other ):
      return(
         (self.name() == other.name() and self.name().find('Twin') != -1) or
         (self.name().split(' ')[0] != 'Krain' and other.name().split(' ')[0] != 'Krain')
      )

   def size_name( self, doubled = False ):
      return self.find('size')

   def size( self, doubled = False ):
      res = self.find('size')
      for i, k in enumerate(['small', 'medium', 'large']):
         if res == k:
            return 2*i + (2 if doubled else 1)

   def stack( self, other = None ):
      utd = self._uptodate
      if other is None:
         self.short = self.shortname() + ' x1'
      elif self.shortname() == other.shortname():
         self.short = self.shortname() + ' x2'
      else:
         self.short = shorten(self.shortname()) + ' + ' + shorten(other.shortname())

      if other:
         sec = {k: v for d, k, v in other.equipped(sec = True)}
         el2 = '$engine_limit' in sec and sec['$engine_limit'] or 0
      else:
         sec = {}
         el2 = 0

      el1 = None
      for d, k, v in self.equipped(sec = False):
         if k == '$engine_limit':
            el1 = v
            break

      done = set()
      for d, k, v in self.equipped(sec = False):
         e = sec[k] if k in sec else 0
         d[k] = prisec(k.lstrip('$'), v, e, el1, el2)
         done.add(k)

      e = self.find('specific')
      for d, k, v in other.equipped(sec = True) if other else []:
         if k not in done:
            e[k] = prisec(k.lstrip('$'), 0, v, el1, el2)
      self._uptodate = utd
      return self

   def equipped( self, sec = False):
      pri_sec = ('sec', 'pri') if sec else ('pri', 'sec')
      for d, k in self.nodes():
         D, K = d.parent(), d.tag()
         if k[-3:] == pri_sec[1]:
            if pri_sec[0] not in d:
               yield D, '$' + K, 0
         elif k[-3:] == pri_sec[0]:
            yield D, '$' + K, d[k]
         elif k[:1]=='$' and k[1:2]!='@':
            yield d, k, d[k]

   def to_dict( self ):
      pri = ((k[1:], v) for (_d, k, v) in self.equipped(sec = False))
      sec = ((k[1:], v) for (_d, k, v) in self.equipped(sec = True))
      return {k: ((v1, v2) if v1!=v2 else v1) for ((k, v1), (_, v2)) in zip(pri, sec)}

   def name( self ):
      return self['outfit']['@name']

   def set_name( self, name ):
      self['outfit']['@name'] = name

   def shortname( self ):
      if not self.short:
         if (res := self.find('shortname')) is None:
            res = self.name()
         if res.split(' ')[-1] == 'Engine':
            res = ' '.join(res.split(' ')[:-1])
         self.short = res
      return self.short

   def prisec_only(self, sec = False):
      for d, k, v in list(self.equipped(sec)):
         d[k] = v

   def save( self ):
      if self.is_multi:
         # make a deep copy
         out = naev_xml()
         out.save_as(self._filename)
         oout = out['outfit'] = self['outfit']

         ind = 3*' '
         lua_inline_mcarg = '{\n'
         for k, v in self.to_dict().items():
            if k in KEEP_IN_XML:
               continue
            if not isinstance(v, tuple):
               v = (v,)
            lua_inline_mcarg += ind + '{' + ', '.join(["'"+k+"'"] + [str(u) for u in v]) + '},\n'
            del oout['specific'][k]
         lua_inline_mcarg += '}'
         lua_inline_mcargs = [lua_inline_mcarg]
         if 'multicore_args' in oout['specific']:
            L = oout['specific']['multicore_args']
            del oout['specific']['multicore_args']
            lua_inline_mcargs += L if isinstance(L, list) else [L]
         lua_inline = '\nrequire("outfits.lib.multicore").init(' + ', '.join(lua_inline_mcargs)
         lua_inline += ')'
         oout['specific']['lua_inline'] = '\n' + (oout['specific']['lua_inline'] + '\n' + lua_inline).strip()
         if 'lua_inline_post' in oout['specific']:
            oout['specific']['lua_inline'] += '\n' + oout['specific']['lua_inline_post'].strip() + '\n'
            del oout['specific']['lua_inline_post']
      else:
         out = self
      naev_xml.save(out)
      self._uptodate = True

if __name__ == '__main__':
   import sys
   for i in sys.argv[1:]:
      o = outfit(i)
      o.touch()
      o.save()
