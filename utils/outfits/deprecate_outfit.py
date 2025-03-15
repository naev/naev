#!/usr/bin/python

from sys import argv,stderr
import xml.etree.ElementTree as ET


DEPR_STR=' (deprecated)'

def main(arg):
   T=ET.parse(arg)
   print "<"+arg+">","deprecated."
   R=T.getroot()
   if R.tag=="outfit":
      R.attrib['name']=R.attrib['name'].split(DEPR_STR,1)[0]+DEPR_STR
   else:
      print >>stderr,"not an outfit:",R.tag
   T.write(arg)

if __name__ == '__main__':
   if '-h' in argv[1:] or '--help' in argv[1:] or len(argv)<2:
      nam=argv[0].split('/')[-1]
      print "usage:",nam,'<outfit.xml>'
      print '  Adds "'+DEPR_STR+'"',"to the name of the outfit"
   else:
      ign=argv[2:]
      if ign!=[]:
         print >>stderr,'Ignored: "'+'", "'.join(ign)+'"'

      main(argv[1])

