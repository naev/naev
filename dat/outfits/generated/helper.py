import sys
from os import path
script_dir = path.join(path.dirname(__file__), '..', '..', '..', 'utils')
sys.path.append(path.realpath(script_dir))
sys.path.append(path.realpath(path.join(script_dir, 'outfits')))
from outfit import outfit, KEEP_IN_XML

INPUT = sys.argv[1]
OUTPUT = sys.argv[2]

def read():
   o = outfit(INPUT)
   o.save_as(OUTPUT)
   if o.is_multi:
      o['outfit']['specific']['multicore_args'] = ['require("outfits.lib.set").set']
   return o
