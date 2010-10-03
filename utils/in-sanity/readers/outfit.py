# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

import os,sys
from readers import readers

class outfit(readers):
    def __init__(self, **config):
        outfitXml = os.path.join(config['datpath'], 'outfit.xml')
        readers.__init__(self, outfitXml, config['verbose'])

        self.outfitsName = list()
        sys.stdout.write('Compiling outfit list ...')
        for outfit in self.xmlData.findall('outfit'):
            self.outfitsName.append(outfit.attrib['name'])
        print "        DONE"

    def find(self, name):
        if name in self.outfitsName:
            return True
        else:
            return False


