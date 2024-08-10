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
d = {'name':name}
tags_recursive( root, d, '' )
#print( d )

outstr = f"""---
title: "{d['name']}"
cond: "return ship.get(\\\"{d['name']}\\\"):known()"
---
<% s = ship.get([[{d['name']}]]) %>
"""
# We don't want any substitution below if possible
outstr += """
## <%= s:name() %>

* **[Class](mechanics/class)**:   <%= _(s:classDisplay()) %>
* **Fabricator**:   <%= _(s:fabricator()) %>

<%= s:description() %>

## Availability

Places where the <%= s:name() %> is sold are shown in #Fgreen#0.
<% function map ( mw )
    local m = require "luatk.map"
    return m.newMap( nil, 10, 0, mw-200, (mw-200) * 9 / 16, {
        binaryhighlight = function ( sys )
            for k,spb in ipairs(sys:spobs()) do
                if spb:known() and inlist( spb:shipsSold(), s ) then
                    return true
                end
            end
            return false
        end
    } )
end %>
<widget map/>

### Ship Properties

* **[Crew](mechanics/boarding)**:   <%= fmt.number(s:crew()) %>
<% if naev.player.fleetCapacity() > 0 then %>
* **[Fleet Capacity](mechanics/playerfleet)**:   <%= fmt.number(s:points()) %>
<% end %>
* **[Mass](mechanics/movement)**:   <%= fmt.f(_("{mass} {unit}"), {mass=fmt.number(s:mass()),unit=naev.unit('mass')}) %>
* **[Base Armour](mechanics/damage)**:   <%= fmt.f(_("{armour} {unit}"), {armour=fmt.number(s:armour()), unit=naev.unit('energy')}) %>
* **[Cargo Space](mechanics/cargo)**:   <%= fmt.f(_("{cargo} {unit}"), {cargo=fmt.number(s:cargo()), unit=naev.unit('energy')})%>
* **[Fuel Consumption](mechanics/hyperspace)**:   <%= fmt.f(_("{fuel} {unit}"), {fuel=fmt.number(s:fuelConsumption()), unit=naev.unit('energy')})%>
* **[Price](mechanics/credits)**:   <%= fmt.credits(s:price()) %>
<% if s:license() then %>
* **License**:   <%= s:license() %>
<% end %>

<%= s:shipstatDesc() %>
"""

with open( args.o, 'w' ) as f:
    f.write( outstr )
