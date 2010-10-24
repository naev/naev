# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

import os,sys
from readers import readers

class unidiff(readers):
    used = list()

    def __init__(self, **config):
        uXml = os.path.join(config['datpath'], 'unidiff.xml')
        readers.__init__(self, uXml, config['verbose'])

        self.uName = list()
        print('Compiling unidiff ...',end='      ')
        for diff in self.xmlData.findall('unidiff'):
            self.uName.append(diff.attrib['name'])
        print("DONE")

    def find(self, name):
        if name in self.uName:
            if name not in self.used:
                self.used.append(name)
            return True
        else:
            return False

    def show_unused(self):
        tmp = self.uName
        for name in self.used:
            tmp.remove(name)

        if len(tmp) > 0:
            print('\nUnused unidiff:')
            print(' '.join(tmp))

