# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

import os,sys
from readers import readers

class fleet(readers):
    used=list()
    unknown=list()

    def __init__(self, **config):
        fleetXml = os.path.join(config['datpath'], 'fleet.xml')
        readers.__init__(self, fleetXml, config['verbose'])
        self._componentName = 'fleet'

        self.nameList = list()
        print('Compiling fleet list ...',end='      ')
        for fleet in self.xmlData.findall('fleet'):
            self.nameList.append(fleet.attrib['name'])
        print("DONE")

    def find(self, name):
        if name in self.nameList:
            if name not in self.used:
                self.used.append(name)
            return True
        else:
            return False


