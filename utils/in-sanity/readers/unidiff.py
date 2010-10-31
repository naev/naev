# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

import os,sys
from readers import readers

class unidiff(readers):
    used = list()
    unknown = list()

    def __init__(self, **config):
        uXml = os.path.join(config['datpath'], 'unidiff.xml')
        readers.__init__(self, uXml, config['verbose'])

        self.uName = list()
        print('Compiling unidiff ...',end='      ')
        for diff in self.xmlData.findall('unidiff'):
            self.uName.append(diff.attrib['name'])
        print("DONE")

    def find(self, name):
        """
        return True if name is found in unidiff.xml
        And if so, add name in the used list.
        """
        if name in self.uName:
            if name not in self.used:
                self.used.append(name)
            return True
        else:
            return False

    def set_unknown(self, name):
        """
        Set the name in an unknown status.
        Meaning it is probably used by a lua script, but this tool can't be
        certain (i.e. name used in a variable).
        """
        if name in self.uName and not in self.used:
            self.unknown.append(name)
            self.used.append(name)

    def get_unused(self):
        tmp = self.uName
        for name in self.used:
            tmp.remove(name)
        return tmp

    def show_unused(self):
        tmp = self.get_unused()
        if len(tmp) > 0:
            print('\nUnused unidiff:')
            print(' '.join(tmp))

