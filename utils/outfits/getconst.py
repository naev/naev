#!/usr/bin/python

import os
import sys
import re

script_dir = os.path.dirname( __file__ )
const_dir = os.path.realpath(os.path.join( script_dir, '..', '..', 'dat', 'constants.lua' ))

try:
   with open(const_dir, 'rt') as fp:
      p = re.compile('PHYSICS_SPEED_DAMP *= *(.*),')
      m = p.search(fp.read())
      s = m.group(1)
except:
   sys.stderr.write('Err: '+str(const_dir)+' not found!\n')
   sys.exit(1)

try:
   f = eval(s)
except:
   sys.stderr.write("Can't read value after \"PHYSICS_SPEED_DAMP = \" in "+str(const_dir)+'.\n')
   sys.exit(1)

PHYSICS_SPEED_DAMP = f

if __name__ == '__main__':
   sys.stderr.write('PHYSICS_SPEED_DAMP = ' + str(f) + '\n')
