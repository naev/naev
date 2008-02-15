#!/usr/bin/env python
"""
See Licensing and Copyright notice in resedit.py
"""

import gtk,gtk.glade
import gobject

import data



class Factions:
   def __init__(self):
      self.factions = {}
      self.factionsXML = "../../dat/faction.xml"
      self.glade = "space.glade"

   def loadFactions(self, xmlfile=None):
      if xmlfile == None:
         xmlFile = self.factionsXML
      self.factions = data.load( xmlfile, "faction", True )
     

   def saveFactions(self, xmlfile=None):
      """
      needs to take into account alliances to be implemented properly
      """
      print "TODO"

  
   def data(self):
      return self.factions


   def window(self):
      self.wtree = gtk.glade.XML(self.glade, "winFactions")

      hooks = { "treFactions":["button-release-event", self.__update]
      }
      for key, val in hooks.items():
         self.__widget(key).connect(val[0],val[1])

      self.__widget("winFactions").show_all()

   def __widget(self, name):
      return self.wtree.get_widget(name)

   def __update(self):
      print "TODO"



   def debug(self):
      print "---FACTIONS----------------"
      print self.factions
      print "---------------------------"
