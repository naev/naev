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
cond: "return ship.get([[{d['name']}]]):known() and not ship.get([[{d['name']}]]):tags().nonaevpedia"
---
<% s = ship.get([[{d['name']}]]) %>
"""
# We don't want any substitution below if possible
outstr += """
<%
local lg = require "love.graphics"
local luatk = require "luatk"
function mainimg ( mw )
   return luatk.newImage( nil, -10, 0, 256, 256, lg.newImage(s:gfxStore()), {
      frame = {0.05,0.05,0.05},
      bkg = {0,0,0},
   } )
end
function rightstats ( mw )
   return luatk.newText( nil, -10, 0, 256, nil, s:shipstatDesc() )
end
function wgtdescription ( mw, tw )
   return luatk.newText( nil, 0, 0, tw, nil, s:description() )
end
%>
<widget mainimg />

<widget rightstats />

# <%= s:name() %>

<widget wgtdescription />

## Properties

* **[Class](ships/classes)**:   <%= _(s:classDisplay()) %>
* **Fabricator**:   <%= _(s:fabricator()) %>
* **[Crew](mechanics/boarding)**:   <%= fmt.number(s:crew()) %>
<% if naev.player.fleetCapacity() > 0 then %>
* **[Fleet Capacity](mechanics/playerfleet)**:   <%= fmt.number(s:points()) %>
<% end %>
* **[Mass](mechanics/movement)**:   <%= fmt.f(_("{mass} {unit}"), {mass=fmt.number(s:mass()),unit=naev.unit('mass')}) %>
* **[Base Armour](mechanics/damage)**:   <%= fmt.f(_("{armour} {unit}"), {armour=fmt.number(s:armour()), unit=naev.unit('energy')}) %>
* **[Cargo Space](mechanics/cargo)**:   <%= fmt.f(_("{cargo} {unit}"), {cargo=fmt.number(s:cargo()), unit=naev.unit('mass')})%>
* **[Fuel Consumption](mechanics/hyperspace)**:   <%= fmt.f(_("{fuel} {unit}"), {fuel=fmt.number(s:fuelConsumption()), unit=naev.unit('unit')})%>
* **[Price](mechanics/credits)**:   <%= fmt.credits(s:price()) %>
<% if s:license() then %>
* **License**:   <%= _(s:license()) %>
<% end %>

## Availability

<%
   local availability = {}
   for i,sys in ipairs(system.getAll()) do
      for j,spb in ipairs(sys:spobs()) do
         if spb:known() and inlist( spb:shipsSold(), s ) then
            table.insert( availability, sys )
            break
         end
      end
   end
   table.sort( availability, function ( a, b )
      return a:jumpDist() < b:jumpDist()
   end )
%>
<% if #availability > 0 then %>
Places where the <%= s:name() %> is sold are shown in #ggreen#0.
<% function map ( mw, tw )
   local luatk_map = require "luatk.map"
   local w = math.min( mw-200, tw-20 )
   local m = luatk_map.newMap( nil, 10, 0, w, w * 9 / 16, {
      binaryhighlight = function ( sys )
         if inlist( availability, sys ) then
            return true
         end
         return false
      end
   } )
   if #availability > 0 then
      m:center( availability[1]:pos(), true )
   end
   return m
end %>
<widget map />
<% else %>
You are not aware of anywhere where the <%= s:name() %> is available for purchase.
<% end %>
"""

with open(args.o, 'w', encoding='utf-8', newline='\n') as f:
   f.write( outstr.replace('\r\n','\n') )
