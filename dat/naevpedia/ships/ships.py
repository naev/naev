#!/usr/bin/python

import xml.etree.ElementTree as ET
import argparse

parser = argparse.ArgumentParser( description='Script to generate naevpedia markdown from ship XML files.')
parser.add_argument('path', metavar='PATH', type=str, help='Name of the ship XML file.')
parser.add_argument('-o', type=str, help='Output path.' )

args, unknown = parser.parse_known_args()

tree = ET.parse( args.path )
root = tree.getroot()
name = root.get('name')

outstr = f"""---
title: "{name}"
cond: "return ship.get(\\\"{name}\\\"):known()"
---
## {name}
"""

with open( args.o, 'w' ) as f:
    f.write( outstr )
