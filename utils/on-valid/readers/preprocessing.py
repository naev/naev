# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

__doc__="""
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
from glob import glob
#from . import readers

from ._Readers import readers

class items(readers):
    def __init__(self, **config):
        xmlFile = os.path.join(config['datpath'], config['xml_file'])
        if '*' in xmlFile:
            xmlFile = glob(xmlFile)
        readers.__init__(self, xmlFile, config['verbose'])
        self._unidiff = config['unidiffobj']
        self.usedItem = list()

        self.itemNames = list()
        print('Compiling {0} list ...'.format(config['item']),end='       ')
        try:
            if type(self.xmlData) is not type(list()):
                for item in self.xmlData.findall(config['item']):
                    self.itemNames.append(item.attrib['name'])
            else:
                for itemList in self.xmlData:
                    for item in itemList.findall(config['item']):
                        self.itemNames.append(item.attrib['name'])
        except Exception as e:
            print('FAILED')
            raise e
        else:
            print("DONE")

    def find(self, name):
        raise NotImplementedError("'find' from 'items' superobject " \
                                  "shouldn't be used.")
#        if name in self.itemNames:
#            if name not in self.usedItem:
#                self.usedItem.append(name)
#            return True
#        else:
#            return False

    def show_unused(self):
        """
        Do not call this method before find !
        """
        raise NotImplementedError("'show_unused' from 'items' superobject " \
                                  "shouldn't be used.")
#        tmp = self.itemNames
#        for name in self.usedItem:
#            tmp.remove(name)
#        print('Unused {0}:'.format(config['item']))
#        print(''.join(tmp))

class tech(items):
    def __init__(self, **config):
        config['xml_file'] = 'tech.xml'
        config['item'] = 'tech'
        items.__init__(self, **config)
        self.assets = assets(techItem=self.findItem, **config)

        self.techItems = list()
        for item in self.xmlData.findall('tech/item'):
            if item.text not in self.techItems:
                self.techItems.append(item.text)

        print('techs validation ...')
        self.assets.validateTechs(self.itemNames)

    def findItem(self, name):
        if name in self.techItems or self._unidiff.findTech(name):
            return True
        else:
            return False

class assets(items):
    def __init__(self, techItem, **config):
        config['xml_file'] = 'assets/*.xml'
        config['item'] = 'asset'
        items.__init__(self, **config)
        self.ssys = ssys(**config)
        self.techItem = techItem

        print('assets validation ...')
        self.ssys.validateAssets(self.itemNames)

    def validateTechs(self, techNames):
        techList = list()
        for techs in self.xmlData:
            techs = techs.getroot()
            for tech in techs.findall('tech/item'):
                if tech.text not in techList:
                    techList.append(tech.text)

        for tech in techNames:
            if self.techItem(tech):
                continue
            if tech not in techList and not self._unidiff.findTech(tech):
                print("Warning: tech ''{0}`` not present in the assets xml".format(tech))

class ssys(readers):
    def __init__(self, **config):
        xmlFile = glob(os.path.join(config['datpath'], 'ssys/*.xml'))
        readers.__init__(self, xmlFile, config['verbose'])
        self._unidiff = config['unidiffobj']

    def validateAssets(self, assetNames):
        # TODO check for empty assets[/asset] <- return '\n  ' if empty
        assetList = list()
        for assets in self.xmlData:
            for asset in assets.findall('ssys/assets/asset/'):
                assetList.append(asset.text)

        for asset in assetNames:
            if asset not in assetList and not self._unidiff.findAsset(asset):
                print("Warning: asset ''{0}`` not present in the ssys xml".format(asset))
