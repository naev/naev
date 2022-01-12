# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

import os,sys
import glob
from ._Readers import readers

class unidiff(readers):

    def __init__(self, **config):
        uXml = glob.glob(os.path.join(config['datpath'], 'unidiff/*.xml'))
        readers.__init__(self, uXml, config['verbose'])
        self._componentName = 'unidiff'
        self.used = list()
        self.unknown = list()

        self.nameList = list()
        self.techList = list()
        self.assetList = list()
        print('Compiling unidiff ...',end='      ')
        for diff in self.xmlData:
            diff = diff.getroot()
            self.nameList.append(diff.attrib['name'])
            for add in diff.findall('tech/add'):
                self.techList.append(add.text)
            for spob in diff.findall('system/spob'):
                self.assetList.append(spob.attrib['name'])
        print("DONE")

    def find(self, name):
        """
        return True if name is found in unidiff/*.xml
        And if so, add name in the used list.
        """
        if name in self.nameList:
            if name not in self.used:
                self.used.append(name)
            return True
        else:
            return False

    def findTech(self, name):
        if name in self.techList:
            return True
        else:
            return False

    def findAsset(self, name):
        if name in self.assetList:
            return True
        else:
            return False
