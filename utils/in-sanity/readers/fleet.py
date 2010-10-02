# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

import os

class fleet(readers):
    def __init__(self, xmlFile, config):
        fleetXml = os.path.join(config['datpath'], 'fleet.xml')
        readers.__init__(fleetXml, config['verbose'])
