#!/usr/bin/env python2
# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

import os, sys
import csv
from lxml import etree
from argparse import ArgumentParser
from types import *


def main(config):
    xmlReader = etree.parse(config.xmlfile)

    # TODO define appropriate dialect (excel, excel-tab or own)
    # see http://docs.python.org/library/csv.html#csv-fmt-params
    csvReader = csv.DictReader(open(config.csvfile,'rU'))
    for cLine in csvReader:
        if config.outfit:
            expr = 'outfit[@name="%s"]'
            outfit = xmlReader.find(expr % cLine['name'])
            if type(outfit) is NoneType:
                print('No outfit for ' + cLine['name'])
                exit()
            else:
                for element in outfit.iter():
                    if element.tag in cLine.keys():
                        element.text = cLine[element.tag]
                    if len(element.attrib) > 0:
                        for (k, v) in element.attrib.items():
                            if element.tag == "damage" and k == "type":
                                element.attrib['type'] = cLine['dtype']
                            elif k in cLine.keys():
                                element.attrib[k] = cLine[k]
        elif config.ship:
            expr = 'ship[@name="%s"]'
            ship = xmlReader.find(expr % cLine['name'])
            if type(ship) is NoneType:
                print('No ship named ' + cLine['name'])
                continue
            else:
                for (cKey, cValue) in cLine.iteritems():
                    if cKey == 'name':
                        continue
                    if cKey == 'license' and cValue == '(null)':
                        continue
                    elem = ship.find(cKey)
                    if type(elem) is NoneType:
                        # we need to go deeper (ò_ô)
                        collapse=True
                        for dream in ship.iterchildren():
                            information = dream.find(cKey)
                            if information is not None:
                                information.text = cValue
                                collapse=False
                                break
                        if collapse:
                            print("No tag `%s' for ship %s") % (cKey, cLine['name'])
                        continue
                    elem.text = cValue
        else:
            print('If no outfit, what to do ?')
    if config.outfit or config.ship:
        newxmlname = os.path.join(os.path.dirname(config.xmlfile),
                                'test_' + os.path.basename(config.xmlfile))
        if not os.path.exists(newxmlname):
            open(newxmlname,'w').close()
        print("Since it's beta, please do a diff and validate %s"%newxmlname)
        xmlReader.write( newxmlname )


__version__ = '1.1'

if __name__ == '__main__':
    parser = ArgumentParser(description="""
        Naev csv to xml tool v%s.
    """ % __version__)
    parser.add_argument('--version', action='version',
                        version='%(prog)s '+__version__)
    parser.add_argument('--verbose', action='store_true', default=False,
                        help='Going verbose to see hidden secrets')
    parser.add_argument('--outfit', '-o', action='store_true',
                        help='Use this to operate on outfits')
    parser.add_argument('--ship', '-s', action='store_true',
                        help='Use this to operate on ships')
    parser.add_argument('csvfile',
                        help='Path to csv files directory')
    parser.add_argument('xmlfile',
                        help='Path to xml file')
    args = parser.parse_args()

    args.csvfile = os.path.abspath(args.csvfile)
    args.xmlfile = os.path.abspath(args.xmlfile)

    main(args)
