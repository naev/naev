#!/usr/bin/python

from os import path
from sys import argv,stderr
import xml.etree.ElementTree as ET


classes={'Courier','Fighter','Bomber','Destroyer','Armoured Transport','Freighter','Battleship','Carrier'}
# Not obvious
classes.add('Bulk Freighter')

def get_path(s):
   s=path.dirname(s)

   if s=='':
      return ''
   else:
      return s+path.sep

def nam2fil(s):
   return s.replace(' ','_').replace('-','').replace("'",'').lower()

# Does not manage circular inheritance. Should not happen.
def find_class(arg):
   T=ET.parse(arg)
   R=T.getroot()

   for e in R.findall('./class'):
      return e.text

   if R.attrib.has_key('inherits'):
      return find_class(get_path(arg)+nam2fil(R.attrib['inherits']+".xml"))
   else:
      return ''

def main(arg):
   if not arg.endswith('.xml'):
      return

   T=ET.parse(arg)
   R=T.getroot()

   if R.tag!="ship":
      #print >>stderr,"not a ship:",R.tag
      return

   ok=False
   cls=find_class(arg)

   if cls not in classes:
      #print >>stderr,R.attrib['name'],":","not the right class:",cls
      return

   # OK! so far, so good.
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

