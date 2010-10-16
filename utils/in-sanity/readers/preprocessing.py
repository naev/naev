# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

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

class assets(items):
    def __init__(self, **config):
        config['xml_file'] = 'asset.xml'
        config['item'] = 'asset']
        items.__init__(self, **config)
        self.ssys = ssys(**config)

        print('assets validation ...')
        self.ssys.validateAssets(self.itemNames)

    def validateTechs(self, techNames):
        techs = self.xmlData.findall('asset/tech')
        for tech in techNames:
            if tech not in techNames:
                print('Warning: tech {0} not used by tech.xml'.format(tech))

class ssys(readers):
    def __init__(self, **config):
        xmlFile = os.path.join(config['datpath'], 'ssys.xml')
        readers.__init__(self, xmlFile, config['verbose'])

    def validateAssets(self, assetNames):
        assets = self.xmlData.findall('ssys/assets')
        for asset in assetNames:
            if asset not in assets:
                print('Warning: asset {0} not used by ssys.xml'.format(asset))
