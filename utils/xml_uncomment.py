#!/usr/bin/env python3

from sys import argv, stderr, stdout, exit
from os import path
from pathlib import Path

if __name__ != '__main__':
   raise Exception('I am a teapot')

args = argv[1:]

if '-h' in args or len(args) < 2:
   stderr.write(
      'usage: ' + argv[0] + '<src_path> <dst_path> <file.xml>...\n'
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
      stderr.write('\033[31merror:\033[0m file "' + arg + '" not in "' + src +'"\n')
      exit(1)
   with open(arg, 'r') as fp:
      s = fp.read()
   oarg = path.join(dst, rel)
   Path(path.split(oarg)[0]).mkdir(parents= True, exist_ok= True)
   with open(oarg, 'w') as fp:
      n = s.find('<!--')
      while n!=-1:
         fp.write(s[:n].rstrip(' '))
         s = s[n + 4:]
         n = s.find('-->')
         if n == -1:
            stderr.write('\033[31merror:\033[0m "<!--" not followed by "-->"')
            exit(-1)
         s = s[n + 3:].lstrip(' ')
         n = s.find('<!--')
      fp.write(s)
   stdout.write(oarg + '\n')
