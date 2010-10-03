# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

import sys

class fleet(readers):
    def __init__(self, **config):
        fleetXml = os.path.join(config['datpath'], 'fleet.xml')
        readers.__init__(fleetXml, config['verbose'])

        fleetsName = list()
        sys.stdout.write('Compiling fleet list ...')
        for fleet in self.xmlData.findall('fleet'):
            fleetsName.append(fleet.attrib['name'])
        print "\t\tDONE"

    def find(self, name):
        if name in self.fleetsName:
            return True
        else:
            return False


