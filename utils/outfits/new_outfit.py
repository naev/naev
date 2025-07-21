# python3


import sys
from sys import stderr
from os import path
script_dir = path.join(path.dirname(__file__), '..')
sys.path.append(path.realpath(script_dir))
from xml_name import xml_name as nam2fil
from naev_xml import naev_xml, _xml_node
import re

MOBILITY_PARAMS = {'speed', 'turn', 'accel', 'thrust'}
LOWER_BETTER = {'mass', 'price', 'delay', 'ew_range', 'falloff', 'trackmin', 'trackmax', 'dispersion', 'speed_dispersion', 'energy_regen_malus', 'ew_stealth', 'ew_stealth_timer', 'ew_signature', 'launch_lockon', 'launch_calibration', 'fwd_energy', 'tur_energy', 'ew_track', 'cooldown_time', 'cargo_inertia', 'land_delay', 'jump_delay', 'delay', 'reload_time', 'iflockon', 'jump_warmup', 'rumble', 'ammo_mass', 'time_mod', 'ew_hide', 'launch_reload'}

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

def un_multicore( o ):
   try:
      e = o.find('lua_inline', ref = True)
      L, remains = parse_lua_multicore( e['lua_inline'] )
   except:
      return False

   e['lua_inline'] = remains
   if not e['lua_inline']:
      del e['lua_inline']
   if not (dst := o.find('specific')):
      dst = o['specific'] = {}
   for (k, v1, v2) in L:
      dst[k] = {'pri': v1, 'sec': v2}
   return True

class outfit():
   # None means auto
   def __init__( self, filename, is_multi = None ):
      self.o = naev_xml(filename)
      self.short = None
      self.is_multi = False
      if is_multi or is_multi is None:
         if un_multicore(self.o):
            self.is_multi = True
         elif is_multi:
            raise ValueError('"' + filename +'" is not a valid multicore.')

   def can_pri_sec( self ):
      if self.is_multi:
         return True, True
      else:
         return False, False

   can_pri = lambda self: self.can_pri_sec()[0]
   can_sec = lambda self: self.can_pri_sec()[1]

   def can_alone( self ):
      return True

   def stack( self, other = None ):
      return self

   def to_dict( self, num_only = True ):
      out = {}
      for d, k in self.o.nodes():
         if k[:1] == '$' and k[1:2] != '@':
            if k[1:] not in {'pri', 'sec'}:
               out[k[1:]] = d[k]
            elif k[1:] == 'pri' and 'sec' in set(d):
               out[d.parent()[1]] = (d['pri'], d['sec'])
      return out

   def name( self ):
      return self.o['outfit']['@name']

   def set_name( self, name ):
      self.o['outfit']['@name'] = name

   def shortname( self ):
      if not self.short:
         if (res := self.o.find('shortname')) is None:
            res = self.name()
         if res.split(' ')[-1] == 'Engine':
            res = ' '.join(res.split(' ')[:-1])
         self.short = res
      return self.short

if __name__ == '__main__':
   for i in argv[1:]:
      o = outfit(i)
      o.o.save()
