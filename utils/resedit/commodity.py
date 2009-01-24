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
      temp = data.load( xmlfile, "commodity", True )
      for t in temp:
         try:
            if temp[t]['price'] != None:
               self.commodities[t] = temp[t]
         except:
            continue
     
   def data(self):
      return self.commodities

   def debug(self):
      print "---COMMODITIES----------------"
      print self.commodities
      print "---------------------------"
