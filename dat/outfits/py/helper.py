import sys
from os import path
script_dir = path.join(path.dirname(__file__), '..', '..', '..', 'utils')
sys.path.append(path.realpath(script_dir))
sys.path.append(path.realpath(path.join(script_dir, 'outfits')))
from naev_xml import naev_xml
from core_outfit import core_outfit, outfit

INPUT = sys.argv[1]
OUTPUT = sys.argv[2]

def read():
   o = naev_xml(INPUT)
   o.save_as(OUTPUT)
   return o

def get_outfit_dict( name, core = False ):
   try:
      if core:
         o = core_outfit(name, try_again = True, quiet = True)
      else:
         o = outfit(name)
   except:
      raise Exception('Could not read "' + path.basename(name) + '"')
   return o.to_dict()

def to_multicore_lua( ref, pri_only = True, setfunc = 'nil' ):
    out = """local multicore = require('outfits.lib.multicore').init({"""
    # We operate under the assumption that dictionaries are ordered in python now
    specific = False
    for r in ref:
        if not specific:
            if r=='specific':
                specific = True
        else:
            v = ref[r]
            if type(v)==float:
                out += f'\n   {{"{r}", {v} }},'
            elif not pri_only and len(v) > 1:
                out += f'\n   {{"{r}", {v[0]}, {v[1]} }},'
            else:
                out += f'\n   {{"{r}", {v[0]} }},'
    out += f'\n}}, {setfunc})'
    return out
