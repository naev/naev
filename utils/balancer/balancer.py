#!/usr/bin/python

import argparse
import csv
import glob
import xml.etree.ElementTree as ET


class Balancer:

    def __init__(self):
        self.ignore = []

    def __tags_recursive( self, root, d, fields, ignore, curpath ):
        for tag in root:
            if tag.tag in ignore:
                continue
            dataname = tag.tag if curpath=='' else curpath+'/'+tag.tag
            # has children
            if len(tag):
                self.__tags_recursive( tag, d, fields, ignore, dataname )
            # no children
            else:
                d[dataname] = tag.text
                fields.update( [dataname] )

    def __xml2dict( self, searchpath, ignore=[] ):

        # parse the xml files
        fields  = set()
        data    = {}
        for filename in glob.glob( searchpath ):
            print(filename)
            root = ET.parse( filename ).getroot()

            name = root.get('name')
            d = {'name':name}
            self.__tags_recursive( root, d, fields, ignore, '' )
            data[name] = d

        fields = list(fields)
        fields.sort()
        fields.insert(0,'name')

        return fields, data

    def __dict2xml( self, data, searchpath ):
        for filename in glob.glob( searchpath ):
            tree = ET.parse( filename )
            root = tree.getroot()
            name = root.get('name')

            # iterate over items
            for key,val in data[name].items():
                # skip name and empty cells
                if key=='name' or val=='':
                    continue
                tags = root.findall(key)
                if len(tags) > 1:
                    print(f"Found duplicate tag '{key}' in '{oname}'. Removing!")
                    for i in range(1,len(tags)):
                        root.remove( tags[i] )
                tag = root.find(key)
                if tag==None:
                    # have to add new tag
                    new_tag = ET.SubElement(root,key)
                    new_tag.text = val
                else:
                    # update existing value
                    tag.text = val
            # save the file
            self.__write_tree( tree, filename )

    def __write_tree( self, tree, ofile ):
        # save the file
        tree.write( ofile, encoding="UTF-8", xml_declaration=True )
        # With python 3.9 the line below should work and make everything pretty ;)
        #ET.indent(tree).write( ofile, encoding="UTF-8", xml_declaration=True )


    def xml2csv( self, csvfile ):
        fields, data = self.__xml2dict( searchpath=self.searchpath, ignore=self.ignore )

        # save the outfit data
        with open(csvfile,'w') as f:
            w = csv.DictWriter( f, fieldnames=fields, quotechar='"', quoting=csv.QUOTE_ALL )
            w.writeheader()
            for n,d in data.items():
                w.writerow(d)

    def csv2xml( self, csvfile ):
        # load the outfit data
        data = {}
        with open(csvfile,'r') as f:
            r = csv.DictReader( f, quotechar='"' )
            for row in r:
                data[ row['name'] ] = row

        self.__dict2xml( data, self.searchpath )


class ShipBalancer(Balancer):
    def __init__(self):
        super().__init__()
        self.searchpath = "dat/ships/*.xml"
        self.ignore = "slots"


class OutfitBalancer(Balancer):
    def __init__(self):
        super().__init__()
        self.searchpath = "dat/outfits/**/*.xml"


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Tool to save Naev xml file data to an csv file and viceversa.')
    parser.add_argument('mode', choices=['outfit','ship'], help="Either 'outfit' to process outfits or 'ship' to process ships.")
    parser.add_argument('readwrite', choices=['w','r'], help="Either 'r' to read from the xml or 'w' to write to the xml.")
    parser.add_argument('filename', type=str, help="CSV file to either read from or write to.")
    args = parser.parse_args()

    if args.mode=='outfit':
        balancer = OutfitBalancer()
    else:
        balancer = ShipBalancer()

    if args.readwrite=='w':
        print( 'Writing to XML files from %s!' % args.filename )
        balancer.csv2xml( args.filename )
    else:
        print( 'Reading from XML files to %s!' % args.filename )
        balancer.xml2csv( args.filename )


