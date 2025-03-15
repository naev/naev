#!/usr/bin/python

from sys import argv,stderr
import xml.etree.ElementTree as ET
from os import os.path


classes={'Courier','Fighter','Bomber','Destroyer','Armoured Transport','Freighter','Battleship','Carrier'}
# Not obvious
classes.add('Bulk Freighter')

#TODO: manage inherits in ship fields

def main(arg):
   if not arg.endswith('.xml'):
      return

   T=ET.parse(arg)
   R=T.getroot()

   if R.tag!="ship":
      #print >>stderr,"not a ship:",R.tag
      return

   ok=False
   cls=''
   for e in R.findall('./class'):
      cls=e.text
      if e.text in classes:
         ok=True

   if not ok:
      print >>stderr,R.attrib['name'],":","not the right class:",cls
      return

   #T.write(arg)

if __name__ == '__main__':
   if '-h' in argv[1:] or '--help' in argv[1:] or len(argv)<2:
      nam=path.basename(argv[0])
      print "usage:",nam,'[-r]','<outfit.xml> ...'
      print "  Adds a new secondary core system slot to the ships given in input, "
      print "  provided they have the right size and don't already have one."
      print '   -r  undo deprecation'
   else:
      for arg in argv[1:]:
         main(arg)

