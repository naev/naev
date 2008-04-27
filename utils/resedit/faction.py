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
      self.glade = "factions.glade"
      self.cur_faction = ""

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

      # list the factions
      self.__createTree()

      self.__widget("winFactions").show_all()

   def __widget(self, name):
      return self.wtree.get_widget(name)

   def __createTree(self):
      wgt = self.__widget("treFactions")
      tree = gtk.TreeStore(str)
      for faction in self.factions: # load up the factions
         treenode = tree.append(None, [faction])
      col = gtk.TreeViewColumn('Factions')
      cell = gtk.CellRendererText()
      if wgt.get_column(0):
         wgt.remove_column( wgt.get_column(0) )
      wgt.append_column(col)
      col.pack_start(cell, True)
      col.add_attribute(cell, 'text', 0)
      wgt.set_model(tree)



   def __update(self, wgt=None, event=None):
      self.cur_faction = self.__curFaction()
      if self.cur_faction == "":
         return

      dic = { "entName":self.cur_faction
      }
      for key, value in dic.items():
         self.__widget(key).set_text(str(value))


   def __curFaction(self):
      tree = self.__widget("treFactions")
      model = tree.get_model()
      try:
         iter = tree.get_selection().get_selected()[1]
      except:                                                             
         return ""
      return model.get_value(iter,0)



   def debug(self):
      print "---FACTIONS----------------"
      print self.factions
      print "---------------------------"
