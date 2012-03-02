# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

import os,sys
from glob import glob
from ._Readers import readers

class ship(readers):
    def __init__(self, **config):
        shipsXml = glob(os.path.join(config['datpath'], 'ships/*.xml'))
        readers.__init__(self, shipsXml, config['verbose'])
        self._componentName = 'ship'
        self._tech = config['tech']
        self._fleet = config['fleetobj']
        self.used = list()
        self.unknown = list()

        self.nameList = list()
        self.missingTech = list()
        print('Compiling ship list ...',end='       ')
        try:
            for ship in self.xmlData:
                ship = ship.getroot()
                self.nameList.append(ship.attrib['name'])
                if not self._tech.findItem(ship.attrib['name']):
                    self.missingTech.append(ship.attrib['name'])
                else:
                    self.used.append(ship.attrib['name'])
        except Exception as e:
            print('FAILED')
            raise e
        else:
            print("DONE")

        for ship in list(self.missingTech):
            if self._fleet.findPilots(ship=ship):
                self.missingTech.remove(ship)
                if ship not in self.used:
                    self.used.append(ship)
        self.missingTech.sort()

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

