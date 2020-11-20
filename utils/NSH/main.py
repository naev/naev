#!/usr/bin/env python3
# -*- encoding: utf8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:
# License: X/MIT
# author: Ludovic Bellière AKA. xrogaan

from collections import defaultdict
import os
import glob
import shutil
from datetime import datetime
from optparse import OptionParser

from jinja2 import Environment, FileSystemLoader
from lxml import etree
import yaml

__version__ = "1.0"

class yamlLabelReader:
    def __init__(self, stream):
        self.ydata = yaml.safe_load(stream)
        keys = set(self.ydata['shipstats'])
        labeled = set(self.ydata['statslabel'])
        for label in keys - labeled:
            print("Warning: missing label for shipstats key:", label)
        for label in labeled - keys:
            print("Notice: statslabel is orphan", label)

    def getShipStatsLabels(self, label):
        """
        Custom filter for the template enigne. Translate the xml values in
        pretty form.
        usage: {{ ship.stats|getStatsLabel }}
        """
        labels = self.ydata['shipstats']
        return labels.get(label, label+"(NOTFOUND)")

    def getStatsLabelsLabel(self, label):
        """
        Custom filter for the template engine. Explain the components.
        usage: {{ stat|getStatsLabelsLabel }}
        """
        labelsLabel = self.ydata['statslabel']
        return labelsLabel.get(label, label+"(NOTFOUND)")

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
            self.__xmlData = glob.glob(os.path.join(xmlPath, "ships/*.xml"))

        self.ships = defaultdict(dict)
        self.shipSortBy = defaultdict(dict)
        for ship in self.__xmlData:
            ship = etree.parse(ship).getroot()
            shipName = ship.get('name')
            shipClass = ship.find('class').text

            # We only want to list player-available ships.
            if ship.find('mission') is not None:
                continue

            for details in ship.iterchildren():
                if details.tag in self.__tagsBlacklist:
                    continue

                if details.tag in self.__tagsSortBase:
                    if details.text not in self.shipSortBy[details.tag]:
                        self.shipSortBy[details.tag][details.text] = []

                # seems empty, ignoring. <- perhapse not a good idea
                if not details.text:
                    continue

                # my, my ... You're quite empty. Let's go for the children.
                if '\n  ' in details.text:
                    compiled = defaultdict(list)
                    for subDetails in details.iterchildren():
                        # we're talking about slots
                        if  details.tag == "slots":
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
                            compiled[subDetails.tag] = subDetails.text
                    self.ships[shipName][details.tag] = compiled
                    del(compiled)
                else:
                    self.ships[shipName][details.tag] = details.text

            self.ships[shipName]['name'] = shipName

            if 'movement' not in self.ships[shipName]:
                self.ships[shipName]['movement'] = {'speed': 0, 'thrust': 0, 'turn': 0}

            for i in self.__tagsSortBase:
                self.store_by(i, self.ships[shipName][i], self.ships[shipName])

    def store_by(self, item, shipDetails, shipData):
        self.shipSortBy[item][shipDetails].append(shipData)

    def get_by(self, item, name=None):
        if item == 'name':
            return self.ships[name]
        return self.shipSortBy[item]

    def __iter__(self):
        return iter(self.ships.items())

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
            self.__xmlData = {}
            __xmlData = glob.glob(os.path.join(xmlPath, 'outfits/*/*.xml'))
            for item in __xmlData:
                basename = os.path.basename(item)
                self.__xmlData[basename] = etree.parse(item)

        self.slots = defaultdict(list)

    def _parseOutfit(self, outfitObj):
        outfitName = outfitObj.get('name')
        outfitGeneral, outfitSpecific = {}, {}

        for general in outfitObj.find('general').iterchildren():
            outfitGeneral[general.tag] = general.text

        for specific in outfitObj.find('specific').iterchildren():
            if specific.tag in self.__tagsBlackList:
                continue
            if len(specific.attrib) < 1:
                outfitSpecific[specific.tag] =  specific.text
            else:
                outfitSpecific[specific.tag] =  {
                            'attribs': specific.attrib,
                            'text': specific.text
                            }

        return {
                'name': outfitName,
                'general': outfitGeneral,
                'specific': outfitSpecific
                }

    def groupBySlots(self):
        for item in self.__xmlData.values():
            mySlot = item.findtext('general/slot')
            if mySlot is None:
                mySlot = "NA"
            self.slots[mySlot].append(self._parseOutfit(item.getroot()))

        return self.slots

    def iterSlot(self, mySlotName):
        for slotName, outfit in self.slots:
            if slotName == mySlotName:
                yield outfit['name']



if __name__ == "__main__":


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
    currentPath = os.path.abspath(os.path.curdir)
    storagePath = os.path.abspath(os.path.normpath(arguments[0]))
    tplPath = os.path.abspath(os.path.normpath(cfg.templates))
    naevPath = os.path.abspath(os.path.normpath("../../dat/"))

    date = str( datetime.utcnow().strftime("%c UTC") )

    myLoader = FileSystemLoader(cfg.templates if cfg.templates else tplPath)
    labels = yamlLabelReader(open('labels.yml', 'r'))
    env = Environment(loader=myLoader)
    env.filters['getStatsLabel'] = labels.getShipStatsLabels
    env.filters['getStatsLabelsLabel'] = labels.getStatsLabelsLabel

    # creating ships html
    myTpl = env.get_template('ships_index.html')
    yaarh = harvester(naevPath)
    shipIStore = os.path.normpath(storagePath + '/ships/')
    if not os.path.exists(storagePath):
        os.mkdir(storagePath, 0o755)
    if not os.path.exists(shipIStore):
        os.mkdir(shipIStore, 0o755)
    myTpl.stream(shipList=yaarh.get_by('class'), date=date).dump(shipIStore+'/index.html')
    del(myTpl)

    for (shipName, shipData) in yaarh:
        myTpl = env.get_template('ship.html')
        myPath = os.path.abspath(os.path.normpath("%s/ships/%s.html" % (storagePath,shipName)))
        myTpl.stream(shipName=shipName, shipData=shipData, date=date).dump(myPath)

    # fancy outfits
    myTpl = env.get_template('outfits_index.html')
    panty = fashion(naevPath)
    outfitsStore = os.path.normpath(storagePath + '/outfits/')
    if not os.path.exists(outfitsStore):
        os.mkdir(outfitsStore, 0o755)
    myTpl.stream(outfits=panty.groupBySlots(), date=date).dump(outfitsStore+'/index.html')

    for (slotName, outfitsList) in panty.slots.items():
        for outfitDetails in outfitsList:
            myTpl = env.get_template('outfit.html')
            myStorage = os.path.normpath("%s/outfits/%s.html") % (
                    storagePath,
                    outfitDetails['name']
                )
            myTpl.stream(slotName=slotName, outfitData=outfitDetails,
                    date=date).dump(myStorage)

    for f in glob.glob(currentPath+'/*.css'):
        bname = os.path.basename(f)
        print('Copying css file:', bname, 'in', storagePath)
        shutil.copy(f, storagePath)

    mediapath = storagePath + '/ships/media'
    if not os.path.exists(mediapath):
        os.mkdir(mediapath)
    print('Copying image files...')
    for f in glob.glob(currentPath+'/../../dat/gfx/ship/*/*_comm.png'):
        bname = os.path.basename(f)
        shutil.copy(f, mediapath)
