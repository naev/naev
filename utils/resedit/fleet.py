#!/usr/bin/env python
"""
See Licensing and Copyright notice in resedit.py
"""

import data


class Fleets:
   def __init__(self):
      self.fleets = {}
      self.fleetsXML = "../../dat/fleet.xml"

   def loadFleets(self, xmlfile=None):
      if xmlfile == None:
         xmlFile = self.fleetsXML
      self.fleets = data.load( xmlfile, "fleet", True )
     
   def data(self):
      return self.fleets

   def debug(self):
      print "---FACTIONS----------------"
      print self.fleets
      print "---------------------------"
