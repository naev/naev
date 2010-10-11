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

class assets(items):
    def __init__(self, **config):
        items.__init__(self, **config):

class ssys(readers):
    def __init__(self, **config):
        config['xml_file'] = 'asset.xml'
        config['item'] = 'asset'
        xmlFile = os.path.join(config['datpath'], 'ssys.xml')
        readers.__init__(self, xmlFile, config['verbose'])
        self.assets = assets(**config)

    def findobjects(self):
        assets = self.xmlData.findall('ssys/assets')
        for asset in assets:
            if not self.assets.find(asset):
                print('Warning')
