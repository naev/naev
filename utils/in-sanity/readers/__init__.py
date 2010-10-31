# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

import xml.etree.ElementTree as ET

class readers:
    """
    Master object with verbosity support
    """

    _verbose=None
    xmlData=None
    used = list()
    unknown = list()

    def __init__(self, xmlFile, verbose=False):
        """
        Set verbosity level and load xml file.
        The file must be valid.
        """
        self._verbose=verbose
        self.nameList=list()
        if self.xmlData is None:
            self.xmlData = ET.parse( xmlFile )

    def get_unused(self):
        tmp = self.nameList
        for name in self.used:
            tmp.remove(name)
        return tmp

    def show_unused(self):
        tmp = self.get_unused()
        if len(tmp) > 0:
            print('\nUnused %s name:' % self._componentName)
            print('\n'.join(tmp))

    def set_unknown(self, name):
        """
        Set the name in an unknown status.
        Meaning it is probably used by a lua script, but this tool can't be
        certain (i.e. name used in a variable).
        """
        if name in self.NameList and not in self.used:
            self.unknown.append(name)
            self.used.append(name)

    def v(self, msg):
        if self._verbose:
            print(msg)
        else:
            pass
