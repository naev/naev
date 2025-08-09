#!/usr/bin/env python3

from sys import stdin, stdout, stderr, exit

s = stdin.read()
n = s.find('<!--')
while n!=-1:
   stdout.write(s[:n].rstrip(' '))
   s = s[n + 4:]
   n = s.find('-->')
   if n == -1:
      stderr.write('"<!--" not followed by "-->"')
      exit(1)
   s = s[n + 3:].lstrip(' ')
   n = s.find('<!--')
stdout.write(s)
