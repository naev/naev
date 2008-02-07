#!/usr/bin/env python

try:   
   import gtk,gtk.glade
   import gobject
except:
   print "You do not have python gtk bindings, or you're missing glade libs"
   print "To use resedit you must install them"
   print "http://pygtk.org/ "
   raise SystemExit


import data

class space:

   def __init__(self):
      self.glade = "space.glade"
      self.systemsXML = "../../dat/ssys.xml"
      self.planetsXML = "../../dat/planet.xml"
      self.loadSystems(self.systemsXML)
      self.loadPlanets(self.planetsXML)


   def loadSystems(self, xmlfile):
      self.systems = data.load(xmlfile, "ssys", True, ["jumps","fleets","planets"])


   def saveSystems(self, xmlfile):
      data.save( "ssys.xml", self.systems, "Systems", "ssys", True,
            {"jumps":"jump","fleets":"fleet","planets":"planet"})


   def loadPlanets(self, xmlfile):
      self.planets = data.load(xmlfile, "planet", True, ["commodities"])


   def savePlanets(self, xmlfile):
        data.save( "planet.xml", self.planets, "Planets", "planet", True,
            {"commodities":"commodity"})


   def window(self):
      """
      create the window
      """
      # --------------- SYSTEMS --------------------
      self.swtree = gtk.glade.XML(self.glade, "winSystems")

      # hook events and such
      self.__swidget("winSystems").connect("destroy", self.__done)
      self.__swidget("treSystems").connect("row-activated", self.__update)
      # buttons
      self.__swidget("butDone").connect("clicked", self.__done)
      self.__swidget("butSave").connect("clicked", self.saveSystems)

      # populate the tree
      self.tree_systems = gtk.TreeStore(str)
      for system in self.systems: # load up the planets
         treenode = self.tree_systems.append(None, [system])
         for planet in self.systems[system]['planets']:
            self.tree_systems.append(treenode, [planet])
      col = gtk.TreeViewColumn('Systems')
      cell = gtk.CellRendererText()
      self.__swidget("treSystems").append_column(col)
      col.pack_start(cell, True)
      col.add_attribute(cell, 'text', 0)
      self.__swidget("treSystems").set_model(self.tree_systems)

      # display the window and such
      self.__swidget("winSystems").show_all()

      # ---------------- PLANETS --------------------

      # ---------------------------------------------
      gtk.main()


   def __swidget(self,wgtname):
      """
      get a widget from the winSystems
      """
      return self.swtree.get_widget(wgtname)


   def __update(self, wgt=None, foo=None, bar=None):
      """
      Update the window
      """
      # important thingies
      tree = self.__swidget("treSystems")
      model = tree.get_model()

      # get system
      sel = model.iter_nth_child(None, foo[0])
      sys_name = model.get_value( sel, 0 )
      system = self.systems[sys_name]

      # load it all
      dic = { "inpName":sys_name,
            "spiInterference":system["general"]["interference"],
            "spiAsteroids":system["general"]["asteroids"],
            "spiStars":system["general"]["stars"],
            "labPos":"%s,%s" % (system["pos"]["x"],system["pos"]["y"])
      }
      for key, value in dic.items():
         self.__swidget(key).set_text(value)

      # load jumps

      jumps = gtk.ListStore(str)
      for jump in system["jumps"]: # load up the planets
         treenode = jumps.append([jump])
      col = gtk.TreeViewColumn('Jumps')
      cell = gtk.CellRendererText()
      wgt = self.__swidget("treJumps")
      if wgt.get_column(0):
         wgt.remove_column( wgt.get_column(0) )
      wgt.append_column(col)
      col.pack_start(cell, True)
      col.add_attribute(cell, 'text', 0)
      wgt.set_model(jumps)

   
   def __done(self, widget=None, data=None):
      """
      Window is done
      """
      gtk.main_quit()


   def debug(self):
      print "SYSTEMS LOADED:"
      print self.systems
      print
      print
      print "PLANETS LOADED:"
      print self.planets

