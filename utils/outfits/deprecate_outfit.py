#!/usr/bin/python

from os import path
from sys import argv,stderr
import xml.etree.ElementTree as ET


DEPR_STR=' (deprecated)'

def main(arg,rev):
   if not arg.endswith('.xml'):
      return
   T=ET.parse(arg)
   print >>stderr, "<"+arg+">","un-deprecated." if rev else "deprecated."
   R=T.getroot()
   if R.tag=="outfit":
      R.attrib['name']=R.attrib['name'].split(DEPR_STR,1)[0]
      if not rev:
         R.attrib['name']=R.attrib['name']+DEPR_STR
   else:
      print >>stderr,"not an outfit:",R.tag
   T.write(arg)

if __name__ == '__main__':
   if '-h' in argv[1:] or '--help' in argv[1:] or len(argv)<2:
      nam=path.basename(argv[0])
      print "usage:",nam,'[-r]','<outfit.xml>'
      print '  Adds "'+DEPR_STR+'"',"to the name of the outfit"
      print '   -r  undo deprecation'
   else:
      reverse='-r' in argv[1:]
      args=[s for s in argv[1:] if s!='-r']
      if args[1:]!=[]:
         print >>stderr,'Ignored: "'+'", "'.join(ign)+'"'

      main(args[0],reverse)

