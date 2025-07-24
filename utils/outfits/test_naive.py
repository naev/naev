#!/usr/bin/env python3

from sys import argv, stderr

def parse_it(s):
   while s:
      i = s.find('<')
      if o := s[:i].strip():
         yield o, False
      if i ==- 1:
         break
      s = s[i:]
      j = s.find('>')
      yield s[1:j].rstrip(), True
      s = s[j+1:]

def _parse_att(s):
   while s:
      i = s.find('="')
      att = s[:i]
      s = s[i+2:]
      i = s.find('"')
      yield '@' + att, s[:i]
      s = s[i+1:]
      if s and s[0]!=' ':
         stderr.write("expected ' ' here: " + repr(s) + '\n')
      else:
         s = s[1:]

def _parse(it, closeit):
   d = {}
   for s, f in it:
      if f:
         if s[0] == '/':
            if s[1:] != closeit:
               stderr.write('unmatched <' + str(closeit) + '><' + s + '>' + '\n')
            return d
         elif s[0:2] == '--' and s[-2:]=='--':
            d['#comment'] = s[2:-2]
         else:
            b, a = tuple((s + ' ').split(' ', 1))
            r1 = dict(_parse_att(a)) | _parse(it, b)
            d[b] = r['#text'] if len(r) == 1 and '#text' in r else r
      else:
         d['#text']= s
   return d

def parse(s):
   return _parse(parse_it(s), None)

def unparse( d , indent=0 ):
   out = ''
   for k, v in d:
      out += indent*' ' + '<' + k + '>'
      if isinstance(v, dict):
         out += '\n'
         out += unparse(v, indent+1)
         out += '\n'
      else:
         out += str(v)
      out += indent*' ' + '</' + k + '>'

from xmltodict import *
for a in argv[1:]:
   with open(a, 'r') as fp:
      d = parse(fp.read())
   with open(a, 'w') as fp:
      fp.write(unparse(d, pretty= True, indent= ' ', full_document= False))
