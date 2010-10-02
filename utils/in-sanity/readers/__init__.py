# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

import xml.etree.ElementTree as ET

class readers:
    """
    Master object with verbosity support
    """

    _verbose=None
    xmlData=None

    def __init__(self, xmlFile, verbose=False):
        """
        Set verbosity level and load xml file.
        The file must be valid.
        """
        self._verbose=verbose
        if self.xmlData is None:
            self.xmlData = ET.parse( xmlFile )

    def v(self, msg):
        if self._verbose:
            print msg
        else:
            pass
