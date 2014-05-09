#!/usr/bin/env python2
# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

__version__ = '1.2'

import os, sys
import csv
from lxml import etree
from types import *

def main(config):
    import string
    translationTable = string.maketrans(' ','_')

    # Read each line of the csv file, find the xml file to edit
    # then do a find&replace for each csv column.
    #
    # TODO define appropriate dialect (excel, excel-tab or own)
    # see http://docs.python.org/library/csv.html#csv-fmt-params
    csvReader = csv.DictReader(open(config.csvfile,'rU'))
    for cLine in csvReader:
        name = cLine['name'].translate(translationTable,'-"\'').lower()
        gFilename = os.path.join(config.xmlfiles, name + '.xml')

        if not os.path.isfile(gFilename):
            print("ERROR: file {} doesn't exists.".format(gFilename))
            continue

        xmlReader = etree.parse(gFilename)
        expr = '*/%s'

        for (cKey, cValue) in cLine.iteritems():
            if cKey == 'name':
                continue
            elem = xmlReader.find(expr % cKey)
            if type(elem) is NoneType:
                print('Cannot find tag {} in current file. Process cancelled.'.format(cKey))
                exit()
            elem.text = cValue

        newxmlname = os.path.join(os.path.dirname(gFilename),
                                'test_' + os.path.basename(gFilename))
        if not os.path.exists(newxmlname):
            open(newxmlname,'w').close()
        print("Since it's beta, please do a diff and validate %s"%newxmlname)
        xmlReader.write( newxmlname )


if __name__ == '__main__':
    from argparse import ArgumentParser

    parser = ArgumentParser(description="""
        Naev csv to xml tool v%s.

        This is a very basic tool, do not expect too much.
    """ % __version__)
    parser.add_argument('--version', action='version',
                        version='%(prog)s '+__version__)
    parser.add_argument('--verbose', action='store_true', default=False,
                        help='Going verbose to see hidden secrets')
    parser.add_argument('csvfile',
                        help='Path to csv files directory')
    parser.add_argument('xmlfiles',
                        help='Path to xml files')

    args = parser.parse_args()

    args.csvfile = os.path.abspath(args.csvfile)
    args.xmlfiles = os.path.abspath(args.xmlfiles)

    main(args)
