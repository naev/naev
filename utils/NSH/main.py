#!/usr/bin/env python2
# -*- encoding: utf8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:
# License: X/MIT
# author: Ludovic Bellière AKA. xrogaan

from os import path
from jinja2 import Environment, FileSystemLoader
import glob
try:
    from lxml import etree
except ImportError:
    print("Failed to import lxml's ElementTree")
    exit()

__version__ = "1.0"

class yamlLabelReader:
    def __init__(self, stream):
        import yaml
        self.ydata = yaml.load(stream)
        keys = self.ydata['shipstats'].keys()
        for label in self.ydata['shipstats'].iterkeys():
            if label not in self.ydata['statslabel']:
                print("Warning: missing label for `%s' shipstats key." % label)
        for lablabel in self.ydata['statslabel'].iterkeys():
            if lablabel not in self.ydata['shipstats']:
                print("Notice: `%s' statslabel is orphan." % lablabel)

    def getShipStatsLabels(self, label):
        """
        Custom filter for the template enigne. Translate the xml values in
        pretty form.
        usage: {{ ship.stats|getStatsLabel }}
        """
        labels = self.ydata['shipstats']
        return labels[label] if labels.has_key(label) else label+"(NOTFOUND)"

    def getStatsLabelsLabel(self, label):
        """
        Custom filter for the template engine. Explain the components.
        usage: {{ stat|getStatsLabelsLabel }}
        """
        labelsLabel = self.ydata['statslabel']
        return labelsLabel[label] if labelsLabel.has_key(label) else label+"(NOTFOUND)"

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
            self.__xmlData = glob.glob(path.join(xmlPath, "ships/*.xml"))

        self.ships = dict()
        self.shipSortBy = dict()
        for ship in self.__xmlData:
            ship = etree.parse(ship).getroot()
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

                # seems empty, ignoring. <- perhapse not a good idea
                if not details.text:
                    continue

                # my, my ... You're quite empty. Let's go for the children.
                if '\n  ' in details.text:
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
            self.__xmlData = dict()
            __xmlData = glob.glob(path.join(xmlPath, 'outfits/*/*.xml'))
            for item in __xmlData:
                basename = path.basename(item)
                self.__xmlData[basename] = etree.parse(item)

        self.slots={}

    def _parseOutfit(self, outfitObj):
        outfitName = outfitObj.get('name')
        outfitGeneral, outfitSpecific = {}, {}

        for general in outfitObj.find('general').iterchildren():
            outfitGeneral.update({general.tag: general.text})

        for specific in outfitObj.find('specific').iterchildren():
            if specific.tag in self.__tagsBlackList:
                continue
            if len(specific.attrib) < 1:
                tmp={specific.tag: specific.text}
            else:
                tmp={
                        specific.tag: {
                            'attribs': specific.attrib,
                            'text': specific.text
                            }
                        }
            outfitSpecific.update(tmp)
            del(tmp)

        return {
                'name': outfitName,
                'general': outfitGeneral,
                'specific': outfitSpecific
                }

    def groupBySlots(self):
        for item in self.__xmlData.itervalues():
            mySlot = item.findtext('general/slot')
            if mySlot is None:
                mySlot = "NA"
            if not self.slots.has_key(mySlot):
                self.slots.update({mySlot: []})
            self.slots[mySlot].append(self._parseOutfit(item.getroot()))

        return self.slots

    def iterSlot(self, mySlotName):
        for slotName, outfit in self.slots:
            if slotName == mySlotName:
                yield outfit['name']



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
    currentPath = path.abspath(path.curdir)
    storagePath = path.abspath(path.normpath(arguments[0]))
    tplPath = path.abspath(path.normpath(cfg.templates))
    naevPath = path.abspath(path.normpath("../../dat/"))

    date = str( datetime.utcnow().strftime("%c UTC") )

    myLoader = FileSystemLoader(cfg.templates if cfg.templates else tplPath)
    labels = yamlLabelReader(open('labels.yml', 'r'))
    env = Environment(loader=myLoader)
    env.filters['getStatsLabel'] = labels.getShipStatsLabels
    env.filters['getStatsLabelsLabel'] = labels.getStatsLabelsLabel

    # creating ships html
    myTpl = env.get_template('ships_index.html')
    yaarh = harvester(naevPath)
    shipIStore = path.normpath(storagePath + '/ships/')
    if not path.exists(storagePath):
        mkdir(storagePath, 0755)
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

    for (slotName, outfitsList) in panty.slots.iteritems():
        for outfitDetails in outfitsList:
            myTpl = env.get_template('outfit.html')
            myStorage = path.normpath("%s/outfits/%s.html") % (
                    storagePath,
                    outfitDetails['name']
                )
            myTpl.stream(slotName=slotName, outfitData=outfitDetails,
                    date=date).dump(myStorage)

    from shutil import copy
    cssFiles = glob.glob(currentPath+'/*.css')
    while len(cssFiles) > 0:
        f = cssFiles.pop()
        bname = path.basename(f)
        if not path.exists(storagePath + '/' + bname):
            print('Copying css file: '+bname+' in '+storagePath)
            copy(f, storagePath)
        else:
            pass

