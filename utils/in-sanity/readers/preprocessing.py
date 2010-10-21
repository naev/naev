# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:
"""
This one's got a lot of objects. It intends to create a validation process for
several xml files. The data could be represented that way:
    Techs validity:
    all tech->name must be in assets xml (asset->tech[])
     - tech->name from tech.xml
     - asset->tech[] from asset.xml

    Assets validity:
    all asset->name must be in ssys xml (ssys->assets[])
     - asset->name from asset.xml
     - ssys->assets[] from ssys.xml

Let describe them :
items:
    Used to be a model. It should makes life easier to tech and asset objects.
    The ''find`` and ''show_unused`` methods aren't used here.

tech:
    That one must be instancied from outside, it will initialize all the
    validation process.
    Will use asset to validate itself.
    Then, it will check if tech.xml has got all the ships and outfit available.

asset:
    Will first validate itself with ''ssys``, then will be used by ''tech`` to
    validate techs.

ssys:
    Just used to list assets and be certain they are all used.
    Is called by ''asset``.
"""
import os,sys
from readers import readers

class items(readers):
    def __init__(self, **config):
        xmlFile = os.path.join(config['datpath'], config['xml_file'])
        readers.__init__(self, xmlFile, config['verbose'])
        self.usedItem = list()

        self.itemNames = list()
        print('Compiling {0} list ...'.format(config['item']),end='       ')
        for item in self.xmlData.findall(config['item']):
            self.itemNames.append(item.attrib['name'])
        print("DONE")

    def find(self, name):
        if name in self.itemNames:
            if name not in self.usedItem:
                self.usedItem.append(name)
            return True
        else:
            return False

    def show_unused(self):
        """
        Do not call this method before find !
        """
        tmp = self.itemNames
        for name in self.usedItem:
            tmp.remove(name)
        print('Unused {0}:'.format(config['item']))
        print(''.join(tmp))

class tech(items):
    def __init__(self, **config):
        config['xml_file'] = 'tech.xml'
        config['item'] = 'tech'
        items.__init__(self, **config)
        self.assets = assets(**config)

        print('techs validation ...')
        self.assets.validateTechs(self.itemNames)

        self.techItems = list()
        for item in self.xmlData.findall('tech/item'):
            if item.text not in self.techItems:
                self.techItems.append(item.text)

    def findItem(self, name):
        if name in self.techItems:
            return True
        else:
            return False

class assets(items):
    def __init__(self, **config):
        config['xml_file'] = 'asset.xml'
        config['item'] = 'asset'
        items.__init__(self, **config)
        self.ssys = ssys(**config)

        print('assets validation ...')
        self.ssys.validateAssets(self.itemNames)

    def validateTechs(self, techNames):
        techs = self.xmlData.findall('asset/tech/item')
        techList = list()
        for tech in techs:
            techList.append(tech.text)
        del(techs)

        for tech in techNames:
            if tech not in techList:
                print("Warning: tech ''{0}`` not present in asset.xml".format(tech))

class ssys(readers):
    def __init__(self, **config):
        xmlFile = os.path.join(config['datpath'], 'ssys.xml')
        readers.__init__(self, xmlFile, config['verbose'])

    def validateAssets(self, assetNames):
        assets = self.xmlData.findall('ssys/assets/asset')
        assetList = list()
        for asset in assets:
            assetList.append(asset.text)
        del(assets)

        for asset in assetNames:
            if asset not in assetList:
                print("Warning: asset ''{0}`` not present in ssys.xml".format(asset))
