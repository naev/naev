#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

import os, sys
import csv
from lxml import etree
from argparse import ArgumentParser


def main(config):
    xmlReader = etree.parse(config.xmlfile)

    # TODO define appropriate dialect (excel, excel-tab or own)
    # see http://docs.python.org/library/csv.html#csv-fmt-params
    csvReader = csv.DictReader(open(config.csvfile,'rU'))
    for cLine in csvReader:
        if config.outfit:
            expr = 'outfit[@name="{1}"]'
            outfit = xmlReader.find('expr'.format(cLine['name']))
            for element in outfit.iter():
                if element.tag in cLine.keys():
                    element.text = cLine[element.tag]
                if len(element.attrib) > 0:
                    for (k, v) in element.attrib:
                        if k in cLine.keys():
                            element[k] = cLine[k]
        else:
            print('If no outfit, what to do ?')
    if config.outfit:
        newxmlname = 'test_' + config.xmlfile
        print("Since it's beta, please do a diff and validate "+newxmlname)
        xmlReader.write( 'test_' + config.xmlfile)


__version__ = '1.0'

if __name__ == '__main__':
    parser = ArgumentParser(description="""
        Naev csc to xml tool v%s.
    """ % __version__)
    parser.add_argument('--version', action='version',
                        version='%(prog)s '+__version__)
    parser.add_argument('--verbose', action='store_true', default=False,
                        help='Going verbose to see hidden secrets')
    parser.add_argument('--outfit', '-o', action='store_true',
                        help='Use this to operate on outfits')
    parser.add_argument('csvfile',
                        help='Path to csv files directory')
    parser.add_argument('xmlfile',
                        help='Path to xml file')
    args = parser.parse_args()

    args.csvfile = os.path.abspath(args.csvfile)
    args.xmlfile = os.path.abspath(args.xmlfile)

    main(args)
