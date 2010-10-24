# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

import os,sys
from readers import readers

class fleet(readers):
    used = list()

    def __init__(self, **config):
        fleetXml = os.path.join(config['datpath'], 'fleet.xml')
        readers.__init__(self, fleetXml, config['verbose'])

        self.fleetsName = list()
        print('Compiling fleet list ...',end='      ')
        for fleet in self.xmlData.findall('fleet'):
            self.fleetsName.append(fleet.attrib['name'])
        print("DONE")

    def find(self, name):
        if name in self.fleetsName:
            if name not in self.used:
                self.used.append(name)
            return True
        else:
            return False

    def show_unused(self):
        tmp = self.fleetsName
        for name in self.used:
            tmp.remove(name)

        if len(tmp) > 0:
            print('\nUnused fleets name:')
            print('\n'.join(tmp))

