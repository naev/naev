# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

import os,sys
from readers import readers

class ship(readers):
    def __init__(self, **config):
        shipXml = os.path.join(config['datpath'], 'ship.xml')
        readers.__init__(self, shipXml, config['verbose'])

        self.shipsName = list()
        sys.stdout.write('Compiling ship list ...')
        for ship in self.xmlData.findall('ship'):
            self.shipsName.append(ship.attrib['name'])
        print "        DONE"

    def find(self, name):
        if name in self.shipsName:
            return True
        else:
            return False


