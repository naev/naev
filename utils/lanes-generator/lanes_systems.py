from collections import namedtuple
import math
import numpy as np
import os
import scipy.sparse as sp
import xml.etree.ElementTree as ET

Asset = namedtuple('Asset', 'x y faction population ran')

def parse_pos(pos):
    if pos is None:
        return (None, None)
    elif 'x' in pos.attrib:
        return ( float(pos.attrib['x']), float(pos.attrib['y']) )
    else:
        return ( float(pos.find('x').text), float(pos.find('y').text) )

def readAssets( path ):
    '''Reads all the assets'''
    assets = {}
    
    for fileName in os.listdir(path):
        tree = ET.parse((path+fileName))
        root = tree.getroot()
        
        name = root.attrib['name']
        
        x, y = parse_pos(root.find('pos'))
        
        presence = root.find('presence')
        if presence == None: # Inhabited
            faction = 'nobody'
            population = 0
            ran = 0
        else:
            faction = presence.find('faction').text
            population = float(presence.find('value').text)
            ran = int(presence.find('range').text)

        assets[name] = Asset( x, y, faction, population, ran )
    
    return assets


# Creates anchors to prevent the matrix from being singular
# Anchors are jumpoints, there is 1 per connected set of systems
# A Robin condition will be added to these points and we will check the rhs
# thanks to them because in u (not utilde) the flux on these anchors should
# be 0, otherwise, it means that the 2 non-null terms on the rhs are on
# separate sets of systems.
def createAnchors(): # TODO : understand what's going on with anchors
    anchorSys = [
                 "Alteris",
                 "Flow",
                 "Zied",        # TODO : be sure this one is necessary
                 "Qorel",
                ]

    anchorJps = [
                 "Delta Pavonis",
                 "Aesria",
                 "Pudas",
                 "Firk",
                ]

    anchorAst = [
                 "Darkshed",
                 "Sevlow",
                 "Bon Sebb",
                 "Qorellia",
                ]

    return (anchorSys, anchorJps, anchorAst)


class Systems:
    '''Readable representation of the systems.'''
    def __init__( self, factions, skip_hidden=True, skip_exitonly=True, skip_uninhabited=False ):
        path = '../../dat/ssys/'
        assets  = readAssets( '../../dat/assets/' )

        self.sysdict = {} # This dico will give index of systems
        self.sysnames = [] # List giving the invert of self.sysdict
        self.nodess = [] # This is a list of nodes in systems

        self.jpnames = [] # List of jump points
        self.jpdicts = [] # List of dict (invert of self.jpnames)

        self.autoposs = []
        self.radius = [] # List of radius of systems
        self.xlist = []
        self.ylist = []
        self.presences = [] # List of presences in systems

        self.loc2globs = [] # Gives the global numerotation from local one for nodes
        self.jp2locs = [] # Gives the local numerotation from jump number
        self.ass2g   = [] # Gives the global numerotation from asset one
        self.g2ass   = [] # Gives asset numerotation from global one
        self.g2sys   = [] # Gives the system from global numerotation

        self.sysass = [] # Assets names per system

        nsys = len(os.listdir(path))

        self.anchors = []
        presass = [] # List of presences in assets
        factass = [] # Factions in assets. (I'm sorry for all these asses)

        i = 0 # No of system
        nglob = 0 # Global nb of nodes
        nasg = 0 # Global nb of assest
        for fileName in os.listdir(path):
            tree = ET.parse((path+fileName))
            root = tree.getroot()

            name = root.attrib['name']
            #print(name)
            self.sysdict[name] = i
            self.sysnames.append(name)

            x, y = parse_pos(root.find('pos'))
            self.xlist.append( x )
            self.ylist.append( y )

            general = root.find('general')
            self.radius.append( float(general.find('radius').text) )

            # Store list of nodes (without jumps)
            nodes = []
            presence = [0] * len(factions)
            assts = root.find('assets')
            loc2glob = []
            sysas = []
            nass = 0

            if assts != None:
                aslist = assts.findall('asset')
                for pnt in aslist :
                    asname = pnt.text
                    info = assets[asname]
                    if info.population > 0 and info.x is not None: # Populated, not a virual asset
                            sysas.append(asname)
                            nodes.append( (info.x, info.y) )
                            loc2glob.append(nglob)
                            self.ass2g.append(nglob)
                            self.g2ass.append(nasg)
                            self.g2sys.append(i)
                            presass.append(info.population)
                            factass.append(factions.get(info.faction, -1))
                            nglob += 1
                            nass += 1
                            nasg += 1

            presence = [max(0,j) for j in presence] # Ensure presences are >= 0
            self.presences.append(presence)
            self.sysass.append(sysas)


            # Store jump points.
            jpname = []
            jpdict = {}
            autopos = []
            jp2loc = []
            jumps = root.find('jumps')
            jplist = jumps.findall('jump')
            jjj = 0
            for jpt in jplist :

                hid = jpt.find('hidden')
                if skip_hidden and hid != None:  # Jump is hidden : don't consider it
                    continue

                xit = skip_exitonly and jpt.find('exitonly')
                if xit != None:  # Jump is exit only : don't consider it
                    continue

                nodes.append( parse_pos( jpt.find('pos') ) )
                autopos.append( nodes[-1] == (None, None) )  # If autopos is activated, need a second loop

                jp2loc.append(nass+jjj)
                loc2glob.append(nglob)
                self.g2ass.append(-1)
                self.g2sys.append(i)
                nglob += 1
                jpname.append(jpt.attrib['target'])
                jpdict[jpt.attrib['target']] = jjj

                jjj += 1

            self.autoposs.append(autopos)
            self.jpnames.append(jpname)
            self.jpdicts.append(jpdict)
            self.jp2locs.append(jp2loc)
            self.nodess.append(nodes)
            self.loc2globs.append(loc2glob)
            i += 1

        self.assts = (presass,factass)

        connect = np.zeros((nsys,nsys)) # Connectivity matrix for systems. TODO : use sparse
        anchorSys, anchorJps, anchorAst = createAnchors()

        for i, (jpname, autopos, loc2globi, jp2loci, namei) in enumerate(zip(self.jpnames, self.autoposs, self.loc2globs, self.jp2locs, self.sysnames)):
            #print(namei)
            for j in range(len(jpname)):
                k = self.sysdict[jpname[j]] # Get the index of target
                connect[i,k] = 1 # Systems connectivity
                connect[k,i] = 1

                if (namei in anchorSys) and (jpname[j] in anchorJps): # Add to anchors
                    self.anchors.append( loc2globi[jp2loci[j]] )

                if autopos[j]: # Compute autopos stuff
                    theta = math.atan2( self.ylist[k]-self.ylist[i], self.xlist[k]-self.xlist[i] )
                    x = self.radius[i] * math.cos(theta)
                    y = self.radius[i] * math.sin(theta)
                    self.nodess[i][jp2loci[j]] = (x,y) # Now we have the position

        # Compute distances.
        self.distances = sp.csgraph.dijkstra(connect)

        # Use distances to compute ranged presences
        for i in range(nsys):
            for j in range(nsys):
                sysas = self.sysass[j]
                for k in range(len(sysas)):
#Asset = namedtuple('Asset', 'x y faction population ran')
                    info = assets[sysas[k]] # not really optimized, but should be OK
                    if info.faction in factions:
                        fact = factions[ info.faction ]
                        d = self.distances[i,j]
                        if d <= info.ran:
                            #self.presences[i][fact] += info.population/(2**d)
                            self.presences[i][fact] += info.population / (1+d)

            # TODO maybe : ensure positive presence
