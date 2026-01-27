#!/usr/bin/env python3

import os
from sys import stderr, exit
import re

script_dir = os.path.dirname( __file__ )
const_dir = os.path.realpath(os.path.join( script_dir, '..', '..', 'constants.lua' ))

try:
   with open(const_dir, 'rt') as fp:
      found = re.search('PHYSICS_SPEED_DAMP *= *(.*),', fp.read()).group(1)
except:
   stderr.write('Err: '+str(const_dir)+' not found!\n')
   sys.exit(1)

try:
   PHYSICS_SPEED_DAMP = eval(found)
except:
   stderr.write(
      "Can't read value after \"PHYSICS_SPEED_DAMP = \" in " + str(const_dir) + '.\n' )
   sys.exit(1)

if __name__ == '__main__':
   stderr.write('PHYSICS_SPEED_DAMP = ' + str(PHYSICS_SPEED_DAMP) + '\n')
