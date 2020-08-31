#!/usr/bin/python
# This file reads missions data and maps it

import xml.etree.ElementTree as ET
import pygraphviz as pgv
import glob

i = 0
namdict = {} # Index from name
names = []
dones = []
uniques = [] #
extra_links = [] # List of links given by extra info

for missionfile in glob.glob( '../dat/missions/**/*.lua', recursive=True ):
    print(missionfile)

    with open(missionfile,'r') as f:
        buf = f.read()
    if buf.find('</mission>') < 0:
        continue
    p = buf.find('--]]')
    if p < 0:
        continue
    xml = buf[5:p]

    tree = ET.ElementTree(ET.fromstring(xml))
    misn = tree.getroot()

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
    
    # Read the notes
    notes_done = misn.findall('note_done')
    for nd in notes_done:
        previous = nd.attrib['name']
        extra_links.append( (previous, name, nd.text)  ) # TODO: add the note
    
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
    
for i in range(len(extra_links)):
    link = extra_links[i]
    G.add_edge( link[0], link[1], label=link[2], color='red' )


# TODO: set the layout (I guess subgraph is what we need)
G.graph_attr['label']='Naev missions map. Red edges come from <note_done> nodes from mission xml'
#G.edge_attr['len']='3'
    
G.layout(prog='dot')
#G.layout()
G.draw('missions.png')
