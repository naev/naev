#!/usr/bin/env python
"""
See Licensing and Copyright notice in resedit.py
"""

import data


class Fleets:
   def __init__(self):
      self.fleets = {}
      self.fleetsXML = "../../dat/fleetgroup.xml"

   def loadFleets(self, xmlfile=None):
      if xmlfile == None:
         xmlfile = self.fleetsXML
      self.fleets = data.load( xmlfile, "fleetgroup", True )
     
   def data(self):
      return self.fleets

   def debug(self):
      print "---FACTIONS----------------"
      print self.fleets
      print "---------------------------"
