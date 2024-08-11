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
<% o = outfit.get([[{d['name']}]]) %>
"""
# We don't want any substitution below if possible
outstr += """
<% function mainimg ( mw )
    local lg = require "love.graphics"
    local luatk = require "luatk"
    return luatk.newImage( nil, -10, 0, 256, 256, lg.newImage(o:icon()) )
end %>
<widget mainimg />

## <%= o:name() %>

<%= o:description() %>

### Availability

<%
    local availability = {}
    for i,sys in ipairs(system.getAll()) do
        for j,spb in ipairs(sys:spobs()) do
            if spb:known() and inlist( spb:outfitsSold(), o ) then
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
Places where <%= o:name() %> are sold are shown in #ggreen#0.
<% function map ( mw )
    local luatk_map = require "luatk.map"
    local m = luatk_map.newMap( nil, 10, 0, mw-200, (mw-200) * 9 / 16, {
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
You are not aware of anywhere where <%= o:name() %> are available for purchase.
<% end %>
"""

with open( args.o, 'w' ) as f:
    f.write( outstr )
