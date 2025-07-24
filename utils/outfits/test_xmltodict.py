#!/usr/bin/env python3

from xmltodict import *
from sys import argv

for a in argv[1:]:
   with open(a, 'r') as fp:
      d = parse(fp.read())
   with open(a, 'w') as fp:
      fp.write(unparse(d, pretty= True, indent= ' ', full_document= False))
