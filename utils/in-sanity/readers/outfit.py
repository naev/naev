# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

import os,sys
import glob
from ._Readers import readers

class outfit(readers):

    def __init__(self, **config):
        outfitXml = glob.glob(os.path.join(config['datpath'], 'outfits/*/*.xml'))
        readers.__init__(self, outfitXml, config['verbose'])
        self._componentName = 'outfit'
        self._tech = config['tech']

        self.used = list()
        self.unknown = list()

        self.nameList = list()
        self.missingTech = list()
        print('Compiling outfit list ...',end='     ')
        for outfit in self.xmlData:
            outfit = outfit.getroot()
            self.nameList.append(outfit.attrib['name'])
            if not self._tech.findItem(outfit.attrib['name']):
                self.missingTech.append(outfit.attrib['name'])
            else:
                self.used.append(outfit.attrib['name'])
        self.missingTech.sort()
        print("DONE")

    def find(self, name):
        if name in self.nameList:
            if name in self.missingTech:
                self.missingTech.remove(name)
            if name not in self.used:
                self.used.append(name)
            return True
        else:
            return False

    def showMissingTech(self):
        if len(self.missingTech) > 0:
            print("\noutfit.xml unused content:")
            for name in self.missingTech:
                print("Warning: item ''{0}`` is not found in tech.xml nor " \
                      "lua files.".format(name))
