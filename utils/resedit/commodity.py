#!/usr/bin/env python
"""
See Licensing and Copyright notice in resedit.py
"""

import data


class Commodities:
   def __init__(self):
      self.commodities = {}
      self.commoditiesXML = "../../dat/commodity.xml"

   def loadCommodities(self, xmlfile=None):
      if xmlfile == None:
         xmlFile = self.commoditiesXML
      self.commodities = data.load( xmlfile, "commodity", True )
     
   def data(self):
      return self.commodities

   def debug(self):
      print "---COMMODITIES----------------"
      print self.commodities
      print "---------------------------"
