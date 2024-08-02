#!/usr/bin/python

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
d = {'name':name}
tags_recursive( root, d, '' )
print( d )

outstr = f"""---
title: "{d['name']}"
cond: "return ship.get(\\\"{d['name']}\\\"):known()"
---
## {name}

{d['description']}

* **[Class](mechanics/class)**:   {d['class']}
* **Fabricator**:   {d['fabricator']}
* **[Crew](mechanics/boarding)**:   {d['characteristics/crew']}
<% if naev.player.fleetCapacity() > 0 then %>
* **[Fleet Capacity](mechanics/playerfleet)**:   {d['points']}
<% end %>
* **[Mass](mechanics/movement)**:   {int(d['characteristics/mass']):,} <%= print(naev.unit('mass')) %>
* **[Base Armour](mechanics/damage)**:   {d['health/armour']} <%= print(naev.unit('energy')) %>
* **[Cargo Space](mechanics/cargo)**:   {d['characteristics/cargo']} <%= print(naev.unit('mass')) %>
* **[Fuel Consumption](mechanics/hyperspace)**:   {d['characteristics/fuel_consumption']} <%= print(naev.unit('unit')) %>
* **[Price](mechanics/credits)**:   {int(d['price']):,} ¤
"""

with open( args.o, 'w' ) as f:
    f.write( outstr )
