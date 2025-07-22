import sys
from os import path
script_dir = path.join(path.dirname(__file__), '..', '..', '..', 'utils')
sys.path.append(path.realpath(script_dir))
sys.path.append(path.realpath(path.join(script_dir, 'outfits')))
from naev_xml import naev_xml
from new_outfit import outfit, KEEP_IN_XML

INPUT = sys.argv[1]
OUTPUT = sys.argv[2]

def read():
   o = naev_xml(INPUT)
   o.save_as(OUTPUT)
   return o

def get_outfit_dict( name, core = False ):
   return outfit(name, is_multi = core).to_dict()

def to_multicore_lua( ref, pri_only = True, setfunc = 'nil' ):
   out = """local multicore = require('outfits.lib.multicore').init({"""
   # We operate under the assumption that dictionaries are ordered in python now
   print(ref)
   for r in ref:
      if r in KEEP_IN_XML:
         continue
      v = ref[r]
      if type(v) in {int,  float}:
         out += f'\n   {{"{r}", {v} }},'
      elif not pri_only and len(v) > 1:
         out += f'\n   {{"{r}", {v[0]}, {v[1]} }},'
      else:
         out += f'\n   {{"{r}", {v[0]} }},'
   return out + f'\n}}, {setfunc})'
