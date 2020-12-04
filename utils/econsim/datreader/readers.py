# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

import os
try:
    from lxml import etree as ET
except ImportError:
    try:
        import xml.etree.ElementTree as ET
    except ImportError:
        print("Failed to import ElementTree")

class readers:
    """
    Simple master object
    CAUTION: This has been updated enough to work, but there's already a set of
    similar-looking but newer classes in utils/on-valid/readers.
    """
    _verbose=None
    xmlData=None

    def __init__(self, xmlPath, subdir, verbose=False):
        self._verbose = verbose
        if self.xmlData is None:
            self.xmlData = ET.Element('directory')
            for path, _, fnames in os.walk(os.path.join(xmlPath, subdir)):
                self.xmlData.extend(ET.parse(os.path.join(path, fn)).getroot()
                                    for fn in fnames)

class ssys(readers):
    def __init__(self, datPath, verbose=False):
        readers.__init__(self, datPath, 'ssys', verbose)

        tmp = self.xmlData.findall('ssys')
        self.jumpsList = dict()
        self.assetsList = dict()
        for system in tmp:
            ssysName = system.get('name')
            jumps = [self._processJump(jump) for jump in system.find('jumps')]
            self.jumpsList.update({ssysName: jumps})
            planets = system.find('assets').getchildren()
            assets = [planet.text for planet in planets]
            self.assetsList.update({ssysName: assets})

    def _processJump(self, jumpNode):
#        pos = jumpNode.find('pos')
        return dict({
            'target': jumpNode.get('target'),
#            'pos': dict({'x': pos.get('x'), 'y': pos.get('y')})
            })

    def planetsForSystem(self, systemName):
        """
        Return a list of planets for the system systemName
        """
        if systemName.find('Virtual') == 0 or \
         systemName not in self.assetsList:
            return None
        return self.assetsList.systemName

    def jumpgatesForSystem(self, systemName):
        """
        Return a list of jump gates for the systems systemName
        Format is {'target': Planet, 'pos': {'x': 0, 'y': 0}}
        """
        return self.jumpsList[systemName]


class assets(readers):
    # should be moved elsewhere or loaded externaly for convenience
    tagWhiteList = ('class','population')

    def __init__(self, datPath, verbose=False):
        readers.__init__(self, datPath, 'assets', verbose)

        # we loads all the assets
        tmp = self.xmlData.findall('asset')
        self.assets = dict()

        for asset in tmp:
            self.assets.update({asset.get('name'): dict()})
            # There are not always all tags, so filter !
            #tags = [self.tagAllowed(child.tag) for child in asset]
            for item in asset.iter():
                tag = self.tagAllowed(item.tag)
                if not tag:
                    continue
                # if there is no text, we assume it's a list
                if not item.text:
                    subItems = [subitem.text for subitem in item.iterchildren()]
                    self.assets[asset.get('name')].update({tag: subItems})
                else:
                    self.assets[asset.get('name')].update({tag: item.text})

    def tagAllowed(self, tag):
        if tag in self.tagWhiteList:
            return tag
        return None

    def getPlanetDetails(self, planetName):
        """
        Get details about a planet.
        The details depends on the tagWhitelist.
        Format is {tagName: data}
        """
        if planetName not in self.assets:
            return None
        return self.assets[planetName]

    def getPopulationGreaterThan(self, population):
        myList = list()
        for (planetName, details) in self.assets:
            if population > int(details['population']):
                myList.append(planetName)

        return myList

    def getPlanetByClass(self, planetClass):
        myList = list()
        for (planetName, details) in self.assets:
            if details['class'] == planetClass:
                myList.append(planetName)

        return myList
