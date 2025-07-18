import sys
from xml_outfit import xml_outfit
from os import path
script_dir = path.join(path.dirname(__file__), '..', '..', '..', 'utils', 'outfits')
sys.path.append(path.realpath(script_dir))
from core_outfit import core_outfit
from outfit import outfit

INPUT = sys.argv[1]
OUTPUT = sys.argv[2]

def read():
   o = xml_outfit(INPUT)
   o.save_as(OUTPUT)
   return o

def write( data ):
   data.save()

def mul_i( d, f, v ):
    d['$' + f] = round( d['$' + f] * v )

def add_i( d, f, v ):
    d['$' + f] += v

def mul_f( d, f, v ):
    d['$' + f] *= v

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
