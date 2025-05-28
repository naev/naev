#!/usr/bin/env python3

if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')


from sys import argv, exit, stderr
from os.path import basename
args = argv[1:]

if '-h' in args or '--help' in args or args == []:
   stderr.write('usage:  ' + basename(argv[0]) + '  <file1> ..\n')
   stderr.write('  Relaxes its input xml ssys files.\n')
   exit(0)

from ssys import starmap, sysnam2sys, sys_fil_ET

sm = starmap()

for i in args:
   stderr.write('relax <' + basename(i) + '> (not implemented)\n')
   pass
