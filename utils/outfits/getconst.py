#!/usr/bin/python

import os
import sys

script_dir = os.path.dirname( __file__ )
const_dir = os.path.join( script_dir, '..', '..' , 'dat' , 'constants.lua' )

try:
   fp=open(const_dir,"rt")
   s=fp.read().split("PHYSICS_SPEED_DAMP",1)
   fp.close()
except:
   sys.stderr.write('Err: '+str(const_dir)+' not found!\n')
   sys.exit(1)

if len(s)!=2:
   sys.stderr.write("Can't find \"PHYSICS_SPEED_DAMP\" in "+str(const_dir)+'.\n')
   sys.exit(1)

s=s[1].strip()
if s[0]=='=':
   s=s[1:]

if s[0] not in "012345679+-.":
   sys.stderr.write("Can't find value after \"PHYSICS_SPEED_DAMP =\" in "+str(const_dir)+'.\n')
   sys.exit(1)

for i in range(len(s)):
   if s[i] not in "012345679+-.eE":
      s=s[:i]
      break

try:
   f=float(s)
except:
   sys.stderr.write("Can't read value after \"PHYSICS_SPEED_DAMP=\" in "+str(const_dir)+'.\n')
   sys.exit(1)

PHYSICS_SPEED_DAMP=f

if __name__=="__main__":
   sys.stderr.write("PHYSICS_SPEED_DAMP="+str(f)+'\n')

