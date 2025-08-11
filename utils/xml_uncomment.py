#!/usr/bin/env python3

from sys import argv, stderr, stdout, exit
from os import path
from pathlib import Path

if __name__ != '__main__':
   raise Exception('I am a teapot')

my_name, *args = argv

if '-h' in args or len(args) < 2:
   stderr.write(
      'usage: ' + my_name + '<src_path> <dst_path> <file.xml>...\n'
      'for each <file.xml>, produces an uncommented version that is written to a file\n'
      'that has the same position in <dst_path> that <file.xml> has in <src_path>.\n'
      'The output filename is written to stdout.\n'
      '\n'
      'This script is not intended to be used as is (but can),\n'
      'it is a subroutine of xml_format.\n'
   )
   exit(0)

src, dst, *args = args

for arg in args:
   if (rel := path.relpath(path.realpath(arg), src))[:2] == '..':
      stderr.write('\033[31merror:\033[0m file ' + repr(arg) + ' not in ' + repr(src) + '\n')
      exit(1)

   oarg = path.join(dst, rel)
   Path(path.split(oarg)[0]).mkdir(parents= True, exist_ok= True)
   with open(arg, 'r') as fpr, open(oarg, 'w') as fpw:
      s = fpr.read()
      bef, *aft = s.split('<!--', 1)
      while aft:
         fpw.write(bef.rstrip(' '))
         s, = aft
         bef, *aft = s.split('-->', 1)
         if not aft:
            stderr.write('\033[31merror:\033[0m "<!--" not followed by "-->"')
            exit(-1)
         s, = aft
         s = s.lstrip(' ')
         bef, *aft = s.split('<!--', 1)
      fpw.write(s)
   stdout.write(oarg + '\n')
