# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

import os,sys
from readers import readers

class ship(readers):
    def __init__(self, **config):
        shipXml = os.path.join(config['datpath'], 'ship.xml')
        readers.__init__(self, shipXml, config['verbose'])
        self._componentName = 'ship'
        tech=config['tech']

        self.nameList = list()
        self.missingTech = list()
        print('Compiling ship list ...',end='       ')
        for ship in self.xmlData.findall('ship'):
            self.nameList.append(ship.attrib['name'])
            if not tech.findItem(ship.attrib['name']):
                self.missingTech.append(ship.attrib['name'])
        print("DONE")

    def find(self, name):
        if name in self.nameList:
            if name in self.missingTech:
                self.missingTech.remove(name)
            return True
        else:
            return False

    def showMissingTech(self):
        if len(self.missingTech) > 0:
            print('\nship.xml unused items:')
            for item in self.missingTech:
                print("Warning: item ''{0}`` is not found in tech.xml nor " \
                      "lua files".format(item))

