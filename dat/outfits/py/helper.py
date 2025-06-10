import sys
import xmltodict
from os import path
script_dir = path.join(path.dirname(__file__), '..', '..', '..', 'utils', 'outfits')
sys.path.append(path.realpath(script_dir))
from core_outfit import core_outfit
from outfit import outfit

INPUT = sys.argv[1]
OUTPUT = sys.argv[2]

def read():
    with open(INPUT,'r') as f:
        return xmltodict.parse( f.read() )

def write( data ):
    with open(OUTPUT,'w') as f:
        f.write( tostring(data) )

def tostring( data ):
    return xmltodict.unparse( data, pretty=True )

def mul_i( d, f, v ):
    d[f] = str(round(float(d[f])*v))

def add_i( d, f, v ):
    d[f] = str(int(d[f])+int(v))

def mul_f( d, f, v ):
    d[f] = str(float(d[f])*v)

def get_outfit_dict( name, core = False ):
   try:
      if core:
         o = core_outfit(name, try_again = True, quiet = True)
      else:
         o = outfit(name)
   except:
      raise Exception('Could not read "' + path.basename(name) + '"')
   return o.to_dict()

def to_multicore_lua( ref, pri_only = True, setfunc = "nil" ):
    out = 'require("outfits.lib.multicore").init({'
    # We operate under the assumption that dictionaries are ordered in python now
    specific = False
    for r in ref:
        if not specific:
            if r=='specific':
                specific = True
        else:
            v = ref[r]
            if not pri_only and len(v) > 1:
                out += f'\n   {{"{r}", {v[0]}, {v[1]} }},'
            else:
                out += f'\n   {{"{r}", {v[0]} }},'
    out += f'\n}}, {setfunc})'
    return out
