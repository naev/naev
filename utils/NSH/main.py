#!/usr/bin/env python2
# -*- encoding: utf8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:
# License: X/MIT
# author: Ludovic Bellière AKA. xrogaan

from os import path
from jinja2 import Environment, FileSystemLoader
try:
    from lxml import etree
except ImportError:
    print("Failed to import lxml's ElementTree")
    exit()

__version__ = "1.0"

def getShipStatsLabels(label):
    """
    Custom filter for the template enigne.
    usage: {{ ship.stats|getStatsLabel }}
    """
    labels = {'jump_delay': "Jump Time",
            'jump_range': "Jump Range",
            'cargo_inertia': "Cargo Inertia",
            'jam_range': "Jam Range",
            'ew_detect': "Detection",
            'ew_hide': "Cloaking",
            'heat_dissipation': "Heat Dissipation",
            'launch_rate': "Launch Rate",
            'launch_range': "Launch Range",
            'jam_counter': "Jam Countermeasures",
            'ammo_capacity': "Ammo Capacity",
            'heat_forward': "Heat (Cannon)",
            'damage_forward': "Damage (Cannon)",
            'firerate_forward': "Fire Rate (Cannon)",
            'energy_forward': "Energy Usage (Cannon)",
            'heat_turret': "Heat (Turret)",
            'damage_turret': "Damage (Turret)",
            'firerate_turret': "Fire Rate (Turret)",
            'energy_turret': "Energy Usage (Turret)",
            'nebula_dmg_shield': "Nebula Damage (Shield)",
            'nebula_dmg_armour': "Nebula Damage (Armour)"}
    return labels[label] if labels.has_key(label) else ""

def getStatsLabelsLabel(label):
    labelsLabel = {
            'jump_delay': "Modulates the time it takes to complete a hyperspace jump.",
            'jump_range': "Modulates the distance a ship can be from a jump point when starting a jump.",
            'cargo_inertia': "Modulates the impact that cargo has on manoeuvrability.",
            'jam_range': "Modulates the distance at which jammers can affect incoming projectiles.",
            'ew_detect': "Modulates the ability to detect other ships.",
            'ew_hide': "Modulates the ship's electronic emissions and visibility to other ships.",
            'heat_dissipation': "Modulates the rate at which heat can be dissipated from the ship and weapons.",
            'launch_rate': "Modulates the rate at which projectiles are fired from launchers.",
            'launch_range': "Modulates the distance projectiles travel once fired.",
            'jam_counter': "Modulates the chance for a ship's missiles to resist an enemy's jamming.",
            'ammo_capacity': "Modulates the amount of ammo equipped launchers can hold.",
            'heat_forward': "Modulates the amount of heat that cannons generate.",
            'damage_forward': "Modulates the per-shot damage dealt by cannons.",
            'firerate_forward': "Modulates the fire rate of cannons.",
            'energy_forward': "Modulates the amount of energy required by cannons.",
            'heat_turret': "Modulates the amount of heat that turrets generate.",
            'damage_turret': "Modulates the per-shot damage dealt by turrets.",
            'firerate_turret': "Modulates the fire rate of turrets.",
            'energy_turret': "Modulates the amount of energy required by turrets.",
            'nebula_dmg_shield': "Modulates the amount of damage that the nebula deals to the shield.",
            'nebula_dmg_armour': "Modulates the amount of damage that the nebula deals to armour."
            }
    return labelsLabel[label] if labelsLabel.has_key(label) else ""

class harvester:
    __xmlData=None
    __tagsBlacklist = ['sound', 'GUI']
    __tagsSortBase = ['base_type', 'class', 'price']
    __tagsSlots = ['utility', 'structure', 'weapon']

    # For updates, see ship.c:820
    __classGroup = {
        'heavy': ['carrier', 'cruiser', 'mothership'],
        'medium': ['cruise ship', 'freighter', 'destroyer', 'corvette',
                        'heavy drone', 'armoured transport']
        }

    def __init__(self, xmlPath):
        if self.__xmlData is None:
            self.__xmlData = etree.parse(path.join(xmlPath, "ship.xml"))

        data = self.__xmlData.findall('ship')
        self.ships = dict()
        self.shipSortBy = dict()
        for ship in data:
            shipName = ship.get('name')
            shipClass = ship.find('class').text

            if not self.ships.has_key(shipName):
                self.ships.update({shipName: dict()})

            for details in ship.iterchildren():
                if details.tag in self.__tagsBlacklist:
                    continue

                if details.tag in self.__tagsSortBase:
                    if not self.shipSortBy.has_key(details.tag):
                        self.shipSortBy.update({details.tag: {}})
                    if not self.shipSortBy[details.tag].has_key(details.text):
                        self.shipSortBy[details.tag].update({details.text:[]})

                # my, my ... You're quite empty. Let's go for the children.
                if '\n   ' in details.text:
                    compiled = dict()
                    for subDetails in details.iterchildren():
                        # we're talking about slots
                        if  details.tag == "slots":
                            if not compiled.has_key(subDetails.tag):
                                compiled.update({subDetails.tag: []})

                            if subDetails.text:
                                size=subDetails.text
                            elif shipClass.lower() in self.__classGroup['heavy']:
                                size='Heavy'
                            elif shipClass.lower() in self.__classGroup['medium']:
                                size='Medium'
                            else:
                                size='Light'

                            compiled[subDetails.tag].append(size)
                        else:
                            compiled.update({subDetails.tag: subDetails.text})
                    self.ships[shipName].update({details.tag: compiled})
                    del(compiled)
                else:
                    self.ships[shipName].update({details.tag: details.text})

            self.ships[shipName].update({'name': shipName})

            for i in self.__tagsSortBase:
                self.store_by(i, self.ships[shipName][i], self.ships[shipName])

    def store_by(self, item, shipDetails, shipData):
        self.shipSortBy[item][shipDetails].append(shipData)

    def get_by(self, item, name=None):
        if item == 'name':
            return self.ships[name]
        return self.shipSortBy[item]

    def iter(self):
        return self.ships.iteritems()

class fashion:
    __xmlData = None
    __tagsBlackList = ['gfx_end','gfx','sound']
    __typeBlackList = ['map', 'gui', 'license']

    # TODO: ammo type have specific arguments (missiles have duration)
    # TODO: retrofit this code, we needs several ways to retrieve data and they
    # are all different. So, I'll discard the initial parsing and do a on
    # request walk only. groupBySlot will list only elements who's got a slot
    # subelement. getByType(name) will return a list of the specific type. Each
    # of them are unique. Here is a list: map, gui, license, ammo, modification.
    def __init__(self, xmlPath):
        if self.__xmlData is None:
            self.__xmlData = etree.parse(path.join(xmlPath, "outfit.xml"))

        data = self.__xmlData.findall('outfit')
        self.outfits = {}
        # we have here 2 parts: general and specific
        for outfit in data:
            specificType = outfit.find('specific').get('type')
            if specificType in self.__typeBlackList:
                continue

            outfitName = outfit.get('name')
            outfitGeneral, outfitSpecific = {}, {},

            for general in outfit.find('general').iterchildren():
                outfitGeneral.update({general.tag: general.text})

            for specific in outfit.find('specific').iterchildren():
                if specific.tag in self.__tagsBlackList:
                    continue
                if specific.tag == 'damage':
                    tmp={specific.tag:
                            {   'type': specific.get('type'),
                                'penetrate': specific.get('penetrate'),
                                'text': specific.text}}
                else:
                    tmp={specific.tag: specific.text}
                outfitSpecific.update(tmp)
                del(tmp)
            self.outfits.update({outfitName:
                   {'specific': outfitSpecific,
                    'general': outfitGeneral}})
            del(outfitSpecific,outfitGeneral)

    def groupBySlots(self):
        slot={}
        for outfitName, outfit in self.outfits.iteritems():
            if not slot.has_key(outfit['general']['slot']):
                slot.update({outfit['general']['slot']: []})
            slot[outfit['general']['slot']].append((outfitName, outfit))
        return slot

    def iterSlot(self,slotName):
        for outfitName, outfit in self.outfits.iteritems():
            if outfit['general']['slot'] == slotName:
                yield (outfitName, outfit)



if __name__ == "__main__":
    from os import mkdir
    from optparse import OptionParser
    from datetime import datetime

    usage="Usage: %prog OUTPUTPATH"
    parser = OptionParser(usage=usage, version="%prog "+__version__,
                          description="Nice looking generator for naev ships")
#    parser.add_option("-o", "--output-path", dest="output", metavar="PATH",
#                      help="Path in whitch files goes.")
    parser.add_option("-t", "--template-path", dest="templates",
                      default='./templates', metavar="PATH",
                      help="""Uses template in that PATH
                              instead of the default one""")

    (cfg, arguments) = parser.parse_args()

    if len(arguments) != 1:
        parser.error("A wise man would know where to store the generated files.")
    storagePath = path.abspath(path.normpath(arguments[0]))
    tplPath = path.abspath(path.normpath(cfg.templates))
    naevPath = path.abspath(path.normpath("../../dat/"))

    date = str( datetime.utcnow().strftime("%c UTC") )

    myLoader = FileSystemLoader(cfg.templates if cfg.templates else tplPath)
    env = Environment(loader=myLoader)
    env.filters['getStatsLabel'] = getShipStatsLabels
    env.filters['getStatsLabelsLabel'] = getStatsLabelsLabel

    # creating ships html
    myTpl = env.get_template('ships_index.html')
    yaarh = harvester(naevPath)
    shipIStore = path.normpath(storagePath + '/ships/')
    if not path.exists(shipIStore):
        mkdir(shipIStore, 0755)
    myTpl.stream(shipList=yaarh.get_by('class'), date=date).dump(shipIStore+'/index.html')
    del(myTpl)

    for (shipName, shipData) in yaarh.iter():
        myTpl = env.get_template('ship.html')
        myPath = path.abspath(path.normpath("%s/ships/%s.html" % (storagePath,shipName)))
        myTpl.stream(shipName=shipName, shipData=shipData, date=date).dump(myPath)

    # fancy outfits
    myTpl = env.get_template('outfits_index.html')
    panty = fashion(naevPath)
    outfitsStore = path.normpath(storagePath + '/outfits/')
    if not path.exists(outfitsStore):
        mkdir(outfitsStore, 0755)
    myTpl.stream(outfits=panty.groupBySlots(), date=date).dump(outfitsStore+'/index.html')

    for (outfitName, outfitDetails) in panty.outfits.iteritems():
        myTpl = env.get_template('outfit.html')
        myStorage = path.normpath("%s/outfits/%s.html") % (
                                                            storagePath,
                                                            outfitName
                                                         )
        myTpl.stream(outfitName=outfitName, outfitData=outfitDetails,
                     date=date).dump(myStorage)
