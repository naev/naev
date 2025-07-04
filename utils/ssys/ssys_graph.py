# python3


import os
from sys import stderr, exit
import xml.etree.ElementTree as ET
from ssys import getpath, PATH


def all_ssys( args = None ):
   path = os.path.join(PATH, 'ssys')
   for arg in os.listdir(path):
      if arg[-4:] == '.xml':
         yield arg[:-4], os.path.join(path, arg)

def ssys_pos( ):
   pos = {}
   for bname, filename in all_ssys():
      T=ET.parse(filename).getroot()

      try:
         e = T.find('pos')
         pos[bname] = (e.attrib['x'], e.attrib['y'])
      except:
         stderr.write('no position defined in "' + bname + '"\n')

   return pos

