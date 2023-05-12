from collections import namedtuple
from dataclasses import dataclass
import glob
import math
import numpy as np
import os
import scipy.sparse as sp
import xml.etree.ElementTree as ET


Asset = namedtuple('Asset', 'x y presences')
Faction = namedtuple('Faction', 'generators invisible lane_base_cost lane_length_per_presence useshiddenjumps')
FactionPresence = namedtuple('FactionPresence', 'facname presence')
Generator = namedtuple('Generator', 'faction weight')

@dataclass
class Presence:
    base: float = 0.
    bonus: float = 0.
    range: int = 0.

    @property
    def value(self):
        return self.base + self.bonus

    def __bool__(self):
        return bool(self.base or self.bonus)

    def __add__(self, rhs):
        return Presence(max(self.base, rhs.base), self.bonus + rhs.bonus)

    def __mul__(self, scalar):
        return Presence(self.base * scalar, self.bonus * scalar)

    def positive_part(self):
        return Presence(max(0., self.base), max(0., self.bonus))


def readFactions(path):
    '''Returns a dictionary of Faction values by name. '''
    full = {}

    for fileName in glob.glob(os.path.join(path, '*.xml')):
        tree = ET.parse(path+fileName)
        root = tree.getroot()
        name = root.attrib['name']

        full[name] = Faction(
            [Generator(gen.text, float(gen.attrib['weight'])) for gen in root.findall('generator')],
            bool(root.find('invisible')),
            float(root.findtext('lane_base_cost', 0)),
            float(root.findtext('lane_length_per_presence', 0)),
            bool(root.find('useshiddenjumps')),
        )
    return {n: f for n, f in full.items() if f.lane_length_per_presence and not f.invisible}

def parse_pos(pos):
    if pos is None:
        return (None, None)
    elif 'x' in pos.attrib:
        return ( float(pos.attrib['x']), float(pos.attrib['y']) )
    else:
        return ( float(pos.findtext('x')), float(pos.findtext('y')) )

def readAssets(path):
    '''Returns a dictionary of Asset values by name. '''
    assets = {}

    for fileName in glob.glob(os.path.join(path, '*.xml')):
        tree = ET.parse((path+fileName))
        root = tree.getroot()
        name = root.attrib['name']
        x, y = parse_pos(root.find('pos'))

        assets[name] = Asset(x, y, presences=[
            FactionPresence(
                presence.findtext('faction'),
                Presence(
                    base=float(presence.findtext('base', 0)),
                    bonus=float(presence.findtext('bonus', 0)),
                    range=int(presence.findtext('range', 0)),
                ),
            )
            for presence in root.findall('presence')
        ])

    return assets


class Systems:
    '''Readable representation of the systems.'''
    def __init__( self ):
        path = '../../dat/ssys/'
        assets  = readAssets( '../../dat/spob/' )
        vassets  = readAssets( '../../dat/spob_virtual/' )
        facinfo = readFactions( '../../dat/factions/' )

        self.facnames = list(facinfo)
        self.lane_base_cost    = {i: facinfo[facname].lane_base_cost           for i, facname in enumerate(self.facnames)}
        self.dist_per_presence = {i: facinfo[facname].lane_length_per_presence for i, facname in enumerate(self.facnames)}
        factions = {name: i for (i, name) in enumerate(self.facnames)}

        self.sysdict = {} # This dico will give index of systems
        self.sysnames = [] # List giving the invert of self.sysdict
        self.nodess = [] # This is a list of nodes in systems

        self.jpnames = [] # List of jump points
        hjnames = [] # List of hidden jump points
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
        sysvass = [] # Virtual asset names (conferring faction presence or anti-presence)

        self.presass = [] # List of presences in assets
        self.factass = [] # Factions in assets. (I'm sorry for all these asses)

        i = 0 # No of system
        nglob = 0 # Global nb of nodes
        nasg = 0 # Global nb of assest
        for fileName in sorted(os.listdir(path)):
            tree = ET.parse((path+fileName))
            root = tree.getroot()

            name = root.attrib['name']
            self.sysdict[name] = i
            self.sysnames.append(name)

            x, y = parse_pos(root.find('pos'))
            self.xlist.append( x )
            self.ylist.append( y )
            self.radius.append( float(root.findtext('general/radius')) )

            # Store list of nodes (without jumps)
            nodes = []
            loc2glob = []
            sysas = []
            sysvas = []
            nass = 0

            for pnt in root.findall('spobs/spob'):
                asname = pnt.text
                info = assets[asname]
                assert len(info.presences) <= 1, f'Real asset {asname} unexpectedly has multiple presence elements.'
                for facname, facpres in info.presences:
                    if not facpres:
                        continue
                    sysas.append(asname)
                    nodes.append( (info.x, info.y) )
                    loc2glob.append(nglob)
                    self.ass2g.append(nglob)
                    self.g2ass.append(nasg)
                    self.g2sys.append(i)
                    self.presass.append(facpres)
                    self.factass.append(factions.get(facname, -1))
                    nglob += 1
                    nass += 1
                    nasg += 1
            for virt in root.findall('spobs/spob_virtual'):
                sysvas.append(virt.text)

            presence = [Presence() for _ in factions]
            self.presences.append(presence)
            self.sysass.append(sysas)
            sysvass.append(sysvas)


            # Store jump points.
            jpname = []
            hjname = []
            jpdict = {}
            autopos = []
            jp2loc = []
            jumps = root.find('jumps')
            jplist = jumps.findall('jump')
            jjj = 0
            for jpt in jplist:
                if jpt.find('hidden') is not None:
                    hjname.append(jpt.attrib['target'])
                    continue

                if jpt.find('exitonly') is not None:
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

        nsys = len(self.sysnames)
        connect = np.zeros((nsys,nsys)) # Connectivity matrix for systems.

        for i, (jpname, autopos, loc2globi, jp2loci, namei) in enumerate(zip(self.jpnames, self.autoposs, self.loc2globs, self.jp2locs, self.sysnames)):
            for j in range(len(jpname)):
                k = self.sysdict[jpname[j]] # Get the index of target
                connect[i,k] = 1 # Systems connectivity

                if autopos[j]: # Compute autopos stuff
                    theta = math.atan2( self.ylist[k]-self.ylist[i], self.xlist[k]-self.xlist[i] )
                    x = self.radius[i] * math.cos(theta)
                    y = self.radius[i] * math.sin(theta)
                    self.nodess[i][jp2loci[j]] = (x,y) # Now we have the position

        # Compute distances.
        connect_full = connect.copy()
        for i, hjname in enumerate(hjnames):
            for dest in hjname:
                connect_full[i, self.sysdict[dest]] = 1
        distances_strict = sp.csgraph.dijkstra(connect)
        distances_hidden = sp.csgraph.dijkstra(connect_full)

        # Use distances to compute ranged presences
        for i in range(nsys):
            for j in range(nsys):
                infos = [assets[ai] for ai in self.sysass[j]] + [vassets[ai] for ai in sysvass[j]]
                for info in infos:
                    for facname, facpres in info.presences:
                        if facname not in factions:
                            continue
                        for gen in [Generator(facname, 1.)] + facinfo[facname].generators:
                            if gen.faction not in factions:
                                continue
                            fact = factions[ gen.faction ]
                            d = self.distances_hidden[i,j] if facinfo[gen.faction].useshiddenjumps else distances_strict[i,j]
                            if d <= facpres.range:
                                self.presences[i][fact] += facpres * (gen.weight / (1+d))

        #TODO distinguish between base and bonus? (Cf. same comment in src/safelanes.c)
        for presence in self.presences + [self.presass]:
            presence[:] = [max(0, p.value) for p in presence]
