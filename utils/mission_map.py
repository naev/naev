#!/usr/bin/python
# This file reads missions data and maps it

import xml.etree.ElementTree as ET
import os # Ultimately, we want to use this one
import pygraphviz as pgv


path = '../dat/missions/'
file = '../dat/mission.xml'

tree = ET.parse(file)
root = tree.getroot()

misnl = root.findall('mission')

namdict = {} # Index from name
names = []
dones = []
uniques = [] #

i = 0 # No of mission

for misn in misnl:
    name = misn.attrib['name']
    names.append(name)
    namdict[name] = i
    
    avail = misn.find('avail')
    done = avail.find('done') # TODO: I guess findall is needed if there are more than one
    dones.append(done)
    
    flags = misn.find('flags')
    if flags == None:
        uniques.append(False)
    else:
        unique = flags.find('unique')
        if unique == None:
            uniques.append(False)
        else:
            uniques.append(True)
    # TODO: harder: open the lua code and analyze annotations in there
    
    i += 1

# Generate graph

G=pgv.AGraph(directed=True)
for i in range(len(names)):
    name = names[i]
    if uniques[i]:
        G.add_node(name)
    else:
        G.add_node(name,color='grey')
    
for i in range(len(dones)):
    done = dones[i]
    if done == None:
        continue
    
    name = names[i]
    G.add_edge(done.text,name)


# TODO: set the layout (I guess subgraph is what we need)
G.graph_attr['label']='Naev missions map'
#G.edge_attr['len']='3'
    
G.layout(prog='dot')
#G.layout()
G.draw('missions.png')
