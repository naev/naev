#!/usr/bin/env python3

import xml.etree.ElementTree as ET
import argparse

parser = argparse.ArgumentParser( description='Script to generate naevpedia markdown from ship XML files.')
parser.add_argument('path', metavar='PATH', type=str, help='Name of the ship XML file.')
parser.add_argument('-o', type=str, help='Output path.' )

args, unknown = parser.parse_known_args()

def tags_recursive( root, d, curpath ):
    for tag in root:
        dataname = tag.tag if curpath=='' else curpath+'/'+tag.tag
        # has children
        if len(tag):
            tags_recursive( tag, d, dataname )
        # no children
        else:
            d[dataname] = tag.text

tree = ET.parse( args.path )
root = tree.getroot()
name = root.get('name')
d = {'name':name, 'general/description':""}
tags_recursive( root, d, '' )
#print( d )

outstr = f"""---
title: "{d['name']}"
cond: "return outfit.get(\\\"{d['name']}\\\"):known()"
---
## {name}

{d['general/description']}
"""

with open( args.o, 'w' ) as f:
    f.write( outstr )
