#!/usr/bin/env python3
# This file reads missions data and maps it

# Tier rules so far:

# 0: automatic
# 1: no fight, basic exploration
# 2: occasionnal fight, advanced flying skills needed
# 3: fight needed, light ship and no special skills needed
# 4: Heavy ship needed or tricky combat
# Above: plot-related

# A mission that requires an other mission to be done must be of > tier, or no indicated tier at all

import xml.etree.ElementTree as ET
import pygraphviz as pgv
import glob
import os
import argparse

# Should we represent different campaigns and different tiers?
parser = argparse.ArgumentParser(description='Tool to display the relationships between the missions in Naev.')
parser.add_argument('--campaigns', metavar='c', type=bool, default=False, help="Ignore the campaign relationships when creating the graph." )
parser.add_argument('--tiers', metavar='t', type=bool, default=True, help="Ignore the tier relationships when creating the graph." )
args = parser.parse_args()
ignore_camp = not args.campaigns
ignore_tier = not args.tiers

tierNames = ["Automatic",
             "Basic piloting",
             "Advanced piloting",
             "Basic combat",
             "Advanced combat",
             "Plot-related"]


i = 0
namdict = {} # Index from name
names = []
dones = []
uniques = [] #
extra_links = [] # List of links given by extra info
meta_nodes = [] # List of meta nodes (useful to control how they're displayed)
camp = [] # Name of the campaigns the missions are in
tierL = []

directory = os.path.split(os.getcwd())
if directory[1] == 'utils':
    prefix = '..'
elif directory[1] == 'naev':
    prefix = '.'
else:
    print("Failed to detect where you're running this script from\nPlease enter your path manually")

# Reads all the missions
for missionfile in glob.glob( prefix+'/dat/missions/**/*.lua', recursive=True ):
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

    name = 'Misn: '+misn.attrib['name']
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
    campaign = None
    tier = None
    notes = misn.find('notes')
    if notes != None:
        campaign = notes.find("campaign")
        tier = notes.find("tier")

    if campaign == None:
        campTxt = "Generic Missions"
    else:
        campTxt = campaign.text

    if tier == None:
        tierV = None
    else:
        tierV = int(tier.text)

    campTxt = 'cluster: '+campTxt
    camp.append(campTxt)
    tierL.append(tierV)

    if notes != None:
        done_misn = notes.findall('done_misn')
        for dm in done_misn:
            previous = 'Misn: '+dm.attrib['name']
            if dm.text == None:
                dm.text = ""
            extra_links.append( (previous, name, dm.text)  )

        done_evt = notes.findall('done_evt')
        for dm in done_evt:
            previous = 'Evt: '+dm.attrib['name']
            if dm.text == None:
                dm.text = ""
            extra_links.append( (previous, name, dm.text)  )

        provides = notes.findall('provides')
        for p in provides:
            nextt = p.attrib['name']
            if p.text == None:
                p.text = ""
            extra_links.append( (name, nextt, p.text)  )
            if (not (nextt in meta_nodes)):
                meta_nodes.append((nextt,campTxt))

        requires = notes.findall('requires')
        for r in requires:
            previous = r.attrib['name']
            if r.text == None:
                r.text = ""
            extra_links.append( (previous, name, r.text)  )
            if (not (previous in meta_nodes)):
                meta_nodes.append((previous,campTxt)) # TODO: there will be conflicts between differnent requires

    i += 1

namdictE  = {} # Index from name
namesE    = []
uniquesE  = [] #
campE     = [] # Name of the campaigns the events are in
tierLE     = []

i = 0

# Reads all the Events
for eventfile in glob.glob( prefix+'/dat/events/**/*.lua', recursive=True ):
    print(eventfile)

    with open(eventfile,'r') as f:
        buf = f.read()
    if buf.find('</event>') < 0:
        continue
    p = buf.find('--]]')
    if p < 0:
        continue
    xml = buf[5:p]

    tree = ET.ElementTree(ET.fromstring(xml))
    evt = tree.getroot()

    name = 'Evt: '+evt.attrib['name']
    namesE.append(name)
    namdictE[name] = i

    flags = evt.find('flags')
    if flags == None:
        uniquesE.append(False)
    else:
        unique = flags.find('unique')
        if unique == None:
            uniquesE.append(False)
        else:
            uniquesE.append(True)

    # Read the notes TODO: one function
    campaign = None
    tier = None
    notes = evt.find('notes')

    if notes != None:
        campaign = notes.find("campaign")
        tier = notes.find("tier")

    if campaign == None:
        campTxt = "Generic Events"
    else:
        campTxt = campaign.text

    if tier == None:
        tierV = None
    else:
        tierV = int(tier.text)

    campTxt = 'cluster: '+campTxt
    campE.append(campTxt)
    tierLE.append(tierV)

    if notes != None:
        done_misn = notes.findall('done_misn')
        for dm in done_misn:
            previous = 'Misn: '+dm.attrib['name']
            if dm.text == None:
                dm.text = ""
            extra_links.append( (previous, name, dm.text)  )

        provides = notes.findall('provides')
        for p in provides:
            nextt = p.attrib['name']
            if p.text == None:
                p.text = ""
            extra_links.append( (name, nextt, p.text)  )
            if (not (nextt in meta_nodes)):
                meta_nodes.append((nextt,campTxt))

        done_evt = notes.findall('done_evt')
        for dm in done_evt:
            previous = 'Evt: '+dm.attrib['name']
            if dm.text == None:
                dm.text = ""
            extra_links.append( (previous, name, dm.text)  )

        requires = notes.findall('requires')
        for r in requires:
            previous = r.attrib['name']
            if r.text == None:
                r.text = ""
            extra_links.append( (previous, name, r.text)  )
            if (not (previous in meta_nodes)):
                meta_nodes.append((previous,campTxt))


    i += 1

# Generate graph

G=pgv.AGraph(directed=True)

# Create the tier subgraphs
imax = 0
for v in tierL:
    if v != None:
        if v > imax:
            imax = v
for v in tierLE:
    if v != None:
        if v > imax:
            imax = v

if not ignore_tier:
    for i in range( imax+1 ):
        G.add_subgraph(name=str(i)) # TODO: rationalize the use of int and str
        sub = G.get_subgraph(str(i))
        sub.graph_attr['rank']='same'
        sub.add_node(tierNames[i], shape='octagon')
        if i>0:
            G.add_edge(tierNames[i-1], tierNames[i], style='invis')

# Add meta nodes
for node in meta_nodes:
    name = node[0]
    campagn = node[1]
    #if True:
    if campagn == "cluster: Generic Missions" or campagn == "cluster: Generic Events" or ignore_camp:
        G.add_node(name,shape='hexagon',color='red')
    else:
        sub = G.get_subgraph(campagn)
        if sub is None:
            G.add_subgraph(name=campagn,label=campagn)
            sub = G.get_subgraph(campagn)

        sub.add_node(name,shape='hexagon',color='red')


def subgraph_add( name, subN, tier, unique, subN_check, shape ):
    #if True:
    if subN == subN_check or ignore_camp:
        if unique:
            G.add_node(name,shape=shape)
        else:
            G.add_node(name,shape=shape,color='grey')
    else:
        sub = G.get_subgraph(subN)
        if sub is None:
            G.add_subgraph(name=subN,label=subN)
            sub = G.get_subgraph(subN)

        if unique:
            sub.add_node(name,shape=shape)
        else:
            sub.add_node(name,shape=shape,color='grey')

    if tier != None and (not ignore_tier):
        sub = G.get_subgraph(str(tier))
        sub.add_node(name)


# Missions
for i in range(len(names)):
    subgraph_add( names[i], camp[i], tierL[i], uniques[i], "cluster: Generic Missions", 'ellipse' )

# Same thing for events
for i in range(len(namesE)):
    subgraph_add( namesE[i], campE[i], tierLE[i], uniquesE[i], "cluster: Generic Events", 'box' )


for i in range(len(dones)):
    done = dones[i]
    if done == None:
        continue
    name = names[i]
    G.add_edge('Misn: '+done.text,name)

for i in range(len(extra_links)):
    link = extra_links[i]
    G.add_edge( link[0], link[1], label=link[2], color='red' ) #

#G.graph_attr['rank']='same'
G.layout(prog='dot')
#G.layout(prog='neato')
G.draw('missions.svg')
