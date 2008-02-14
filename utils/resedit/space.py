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

class Space:

   def __init__(self):
      self.glade = "space.glade"
      self.systemsXML = "../../dat/ssys.xml"
      self.planetsXML = "../../dat/planet.xml"
      self.planet_gfx = "../../gfx/planet/"
      self.loadSystems(self.systemsXML)
      self.loadPlanets(self.planetsXML)


   def loadSystems(self, xmlfile):
      self.systems = data.load(xmlfile, "ssys", True,
            ["jumps","planets"], {"fleets":"chance"} )


   def saveSystems(self, xmlfile):
      data.save( "ssys.xml", self.systems, "Systems", "ssys", True,
            {"jumps":"jump","planets":"planet"}, {"fleets":["fleet","chance"]})


   def loadPlanets(self, xmlfile):
      self.planets = data.load(xmlfile, "planet", True, ["commodities"])


   def savePlanets(self, xmlfile):
      data.save( "planet.xml", self.planets, "Planets", "planet", True,
            {"commodities":"commodity"})

   def __genPlanetTree(self):
      self.planetTree = {}
      # set planets to None
      for planet in self.planets.keys():
         self.planetTree[planet] = None
      # set allocated planets
      for name, system in self.systems.items():
         for planet in system["planets"]:
            self.planetTree[planet] = name


   def window(self):
      """
      create the window
      """
      # --------------- SYSTEMS --------------------
      self.swtree = gtk.glade.XML(self.glade, "winSystems")

      # hook events and such
      hooks = { "winSystems":["destroy",self.__done],
            "treSystems":["button-release-event", self.__supdate],
            "inpName":["changed",self.__supdate],
            "butDone":["clicked",self.__done],
            "butSave":["clicked",self.saveSystems],
            "butZoomIn":["clicked",self.__space_zoomin],
            "butZoomOut":["clicked",self.__space_zoomout],
            "butReset":["clicked",self.__space_reset],
            "butAddJump":["clicked",self.__jump_add],
            "butRmJump":["clicked",self.__jump_rm],
            "butNew":["clicked",self.__snew]
      }
      for key, val in hooks.items():
         self.__swidget(key).connect(val[0],val[1])

      self.__create_treSystems()

      # custom widget drawing area thingy
      self.zoom = 1
      self.space_sel = ""
      area = self.__swidget("draSpace")
      area.set_double_buffered(True)
      area.set_events(gtk.gdk.EXPOSURE_MASK
            | gtk.gdk.LEAVE_NOTIFY_MASK
            | gtk.gdk.BUTTON_PRESS_MASK
            | gtk.gdk.POINTER_MOTION_MASK
            | gtk.gdk.POINTER_MOTION_HINT_MASK)
      area.connect("expose-event", self.__space_draw)
      area.connect("button-press-event", self.__space_down)
      area.connect("motion-notify-event", self.__space_drag)

      # display the window and such
      self.__swidget("winSystems").show_all()
      self.cur_system = ""
      self.x = self.y = 0
      self.lx = self.ly = 0

      # ---------------- PLANETS --------------------
      self.pwtree = gtk.glade.XML(self.glade, "winPlanets")

      self.__pwidget("winPlanets").show_all()
      self.cur_planet = ""

      # hooks
      hooks = { "butNew":["clicked",self.__pnew],
            "trePlanets":["button-release-event", self.__pupdate]
      }
      for key, val in hooks.items():
         self.__pwidget(key).connect(val[0],val[1])

      # planet tree
      self.__create_trePlanets()

      # classes
      classes = [ "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L",
            "M", "N", "O", "P", "Q", "R", "S", "T", "X", "Y", "Z", "0", "1" ]
      wgt = self.__pwidget("comClass")
      combo = gtk.ListStore(str)
      for a in classes:
         node = combo.append(a)
      cell = gtk.CellRendererText()
      wgt.pack_start(cell, True)
      wgt.add_attribute(cell, 'text', 0)
      wgt.set_model(combo)

      # ---------------------------------------------
      gtk.main()


   def __create_treSystems(self):
      # populate the tree
      wgt = self.__swidget("treSystems")
      self.tree_systems = gtk.TreeStore(str)
      for system in self.systems: # load up the planets
         treenode = self.tree_systems.append(None, [system])
         for planet in self.systems[system]['planets']:
            self.tree_systems.append(treenode, [planet])
      col = gtk.TreeViewColumn('Systems')
      cell = gtk.CellRendererText()
      if wgt.get_column(0):
         wgt.remove_column( wgt.get_column(0) )
      wgt.append_column(col)
      col.pack_start(cell, True)
      col.add_attribute(cell, 'text', 0)
      wgt.set_model(self.tree_systems)


   def __create_trePlanets(self):
      # population
      wgt = self.__pwidget("trePlanets")
      self.tree_planets = gtk.TreeStore(str)
      for planet in self.planets:
         treenode = self.tree_planets.append(None, [planet])
      col = gtk.TreeViewColumn('Planets')
      cell = gtk.CellRendererText()
      if wgt.get_column(0):
         wgt.remove_column( wgt.get_column(0) )
      wgt.append_column(col)
      col.pack_start(cell, True)
      col.add_attribute(cell, 'text', 0)
      wgt.set_model(self.tree_planets)

   def __swidget(self,wgtname):
      """
      get a widget from the winSystems
      """
      return self.swtree.get_widget(wgtname)
   
   def __pwidget(self,wgtname):
      return self.pwtree.get_widget(wgtname)


   def __supdate(self, wgt=None, index=None, iter=None):
      """
      Update the window
      """

      # store the current values
      self.__sstore()

      self.cur_system = self.__curSystem()
      if self.cur_system == "":
         return
      system = self.systems[self.cur_system]

      # load it all
      dic = { "inpName":self.cur_system,
            "spiInterference":system["general"]["interference"],
            "spiAsteroids":system["general"]["asteroids"],
            "spiStars":system["general"]["stars"],
            "labPos":"%s,%s" % (system["pos"]["x"],system["pos"]["y"])
      }
      for key, value in dic.items():
         self.__swidget(key).set_text(str(value))

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

      self.__space_draw()

   def __pupdate(self, wgt=None, event=None):
      # store current values
      self.__pstore()

      self.__genPlanetTree()

      self.cur_planet = self.__curPlanet()
      if self.cur_planet == "":
         return
      planet = self.planets[self.cur_planet]

      dic = { "inpName":self.cur_planet
      }

      for key,value in dic.items():
         self.__pwidget(key).set_text(str(value))

      # class
      cls = planet["general"]["class"]
      i = 0
      wgt = self.__pwidget("comClass")
      model = wgt.get_model()
      for row in model:
         if row[0] == cls:
            wgt.set_active_iter(model.get_iter(i))
         i = i + 1

      # tech
      try:
         self.__pwidget("spiTech0").set_text(str(planet["general"]["tech"]["main"]))
      except:
         self.__pwidget("spiTech0").set_text(str(0))

      # services
      services = int(planet["general"]["services"])
      serv = { "cheLand":2**0,
            "cheBasic":2**1,
            "cheCommodity":2**2,
            "cheOutfits":2**3,
            "cheShipyard":2**4 }
      for s,m in serv.items():
         if services & m > 0:
            self.__pwidget(s).set_active(True)
         else:
            self.__pwidget(s).set_active(False)

      # image
      self.__pwidget("imaPlanet").set_from_file( self.planet_gfx + "space/" +
            planet["GFX"]["space"] )

      # system
      wgt = self.__pwidget("comSystem")
      combo = gtk.ListStore(str)
      for sysname in self.systems.keys():
         node = combo.append([sysname])
      cell = gtk.CellRendererText()
      if wgt.get_model() == None:
         wgt.pack_start(cell, True)
         wgt.add_attribute(cell, 'text', 0)
      wgt.set_model(combo)
      self.__genPlanetTree()
      i = 0
      for row in combo:
         if row[0] == self.planetTree[self.cur_planet]:
            wgt.set_active_iter(combo.get_iter(i))
         i = i + 1



   def __sstore(self):
      sys_name = self.__swidget("inpName").get_text()
      if sys_name == "":
         return

      # renamed the current system
      if sys_name != self.cur_system:
         self.systems[sys_name] = self.systems[self.cur_system] # copy it over
         model = self.__swidget("treSystems").get_model()

         # must rename the node in the treeview
         for i in model:
            if i[0] == self.cur_system:
               i[0] = sys_name
               break

         # update jump paths
         for key,value in self.systems.items():
            i = 0
            for jump in value["jumps"]:
               if jump == self.cur_system:
                  self.systems[key]["jumps"].pop(i)
                  self.systems[key]["jumps"].append(sys_name)
               i = i+1

         # delete the old system and change current to it
         del self.systems[self.cur_system] # get rid of the old one
         self.cur_system = sys_name # now use self.cur_system again

      try: 
         system = self.systems[self.cur_system] 
      except:
         return # no system selected yet

      # start load all the input stuff
      # general stuff
      self.__sinpStore(system,"spiStars","general","stars") 
      self.__sinpStore(system,"spiInterference","general","interference")
      self.__sinpStore(system,"spiAsteroids","general","asteroids")


   def __pstore(self):
      planet_name = self.__pwidget("inpName").get_text()
      if planet_name == "":
         return

      # changed planet name
      if planet_name != self.cur_planet:
         self.planets[planet_name] = self.planets[self.cur_planet]
         model = self.__pwidget("trePlanets").get_model()
         
         for i in model:
            if i[0] == self.cur_planet:
               i[0] = planet_name
               break

         del self.planets[self.cur_planet]
         self.cur_planet = planet_name

      try:
         planet = self.planets[self.cur_planet]
      except:
         return


      # get the services
      services = 0
      serv = { "cheLand":2**0,
            "cheBasic":2**1,
            "cheCommodity":2**2,
            "cheOutfits":2**3,
            "cheShipyard":2**4 }
      for s,m in serv.items():
         if self.__pwidget(s).get_active():
            services = services + m
      planet["general"]["services"] = services
     

   def __sinpStore(self, system, wgt, tag, minortag=None):
      text = self.__swidget(wgt).get_text()
      if minortag==None:
         system[tag] = text
      else:
         system[tag][minortag] = text



   def __curSystem(self):
      # important thingies
      tree = self.__swidget("treSystems")
      model = tree.get_model()
      try:
         iter = tree.get_selection().get_selected()[1]
         p = model.iter_parent(iter)
      except:
         return ""
      if p == None:
         return model.get_value(iter,0)
      else:
         return model.get_value(p,0)


   def __curPlanet(self):
      tree = self.__pwidget("trePlanets")
      model = tree.get_model()
      try:
         iter = tree.get_selection().get_selected()[1]
      except:
         return ""
      return model.get_value(iter,0)

   
   def __done(self, widget=None, data=None):
      """
      Window is done
      """
      gtk.main_quit()


   def __space_reset(self, wgt=None, event=None):
      self.x = self.y = 0
      self.space_sel = ""
      self.__space_draw()

   def __space_down(self, wgt, event):

      x = event.x
      y = event.y

      wx,wy, ww,wh = self.__swidget("draSpace").get_allocation()

      mx = (x - (self.x*self.zoom + ww/2))/self.zoom
      my = (y - (self.y*self.zoom + wh/2))/self.zoom

      # modify the current position
      if event.button == 1 and self.__swidget("butReposition").get_active() and self.cur_system != "":

         system = self.systems[self.cur_system]
         system["pos"]["x"] = str(int(mx))
         system["pos"]["y"] = str(int(-my))

         self.__space_draw()

         self.__swidget("butReposition").set_active(False)

      # see if a system is in "click" range
      r = 15
      for name, sys in self.systems.items():
         sx = int(sys["pos"]["x"])
         sy = -int(sys["pos"]["y"])

         if abs(sx-mx) < r and abs(sy-my) < r:
            if event.button == 1 and event.type == gtk.gdk._2BUTTON_PRESS:
               tree = self.__swidget("treSystems")
               self.__selSys(name)
            elif event.button == 1:
               self.space_sel = name
               self.__space_draw()
            break



      self.lx = x
      self.ly = y

      return True

   def __space_drag(self, wgt, event):
      x = event.x
      y = event.y
      state = event.state

      wx,wy, ww,wh = self.__swidget("draSpace").get_allocation()

      mx = (x - (self.x*self.zoom + ww/2))/self.zoom
      my = (y - (self.y*self.zoom + wh/2))/self.zoom

      self.__swidget("labCurPos").set_text( "%dx%d" % (mx,my) )

      if state & gtk.gdk.BUTTON1_MASK:
         xrel = x - self.lx
         yrel = y - self.ly
         
         self.x = self.x + xrel/self.zoom
         self.y = self.y + yrel/self.zoom
         self.lx = x
         self.ly = y
         
         self.__space_draw()

      return True

   def __space_zoomout(self, wgt=None, event=None):
      self.zoom = self.zoom/2
      if self.zoom < 0.1:
         self.zoom = 0.1
      self.__space_draw()
   def __space_zoomin(self, wgt=None, event=None):
      self.zoom = self.zoom*2
      if self.zoom > 4:
         self.zoom = 4
      self.__space_draw()


   def __space_draw(self, wgt=None, event=None):
      area = self.__swidget("draSpace")
      wx,wy, ww,wh = area.get_allocation()
      cx = self.x*self.zoom + ww/2
      cy = self.y*self.zoom + wh/2
      r = 15

      # colour stuff
      sys_gc = area.window.new_gc()
      col = area.get_colormap().alloc_color("yellow")
      sys_gc.foreground = col
      bg_gc = area.window.new_gc()
      col = area.get_colormap().alloc_color("black")
      bg_gc.foreground = col
      jmp_gc = area.window.new_gc()
      col = area.get_colormap().alloc_color("blue")
      jmp_gc.foreground = col
      sel_gc = area.window.new_gc()
      col = area.get_colormap().alloc_color("red")
      sel_gc.foreground = col
      inert_gc = area.window.new_gc()
      col = area.get_colormap().alloc_color("grey")
      inert_gc.foreground = col

      # cleanup
      area.window.draw_rectangle(bg_gc, True, 0,0, ww,wh)
      area.window.draw_rectangle(sys_gc, False, 0,0, ww-1,wh-1)

      for sys_name, system in self.systems.items():
         sx = int(system["pos"]["x"])
         sy = -int(system["pos"]["y"])
         dx = int(cx+sx*self.zoom)
         dy = int(cy+sy*self.zoom)

         # draw jumps
         for jump in system["jumps"]:
            jsx = int(self.systems[jump]["pos"]["x"])
            jsy = -int(self.systems[jump]["pos"]["y"])
            jdx = int(cx+jsx*self.zoom)
            jdy = int(cy+jsy*self.zoom)
            
            area.window.draw_line(jmp_gc, dx,dy, jdx,jdy)


         # draw circle
         if sys_name == self.space_sel:
            gc = sel_gc
         elif len(system["planets"]) == 0:
            gc = inert_gc
         else:
            gc = sys_gc
         if sys_name == self.__curSystem():
            gc2 = jmp_gc
         else:
            gc2 = bg_gc
         area.window.draw_arc(gc, True, dx-r/2,dy-r/2, r,r, 0,360*64)
         area.window.draw_arc(gc2, True, dx-r/2+2,dy-r/2+2, r-4,r-4, 0,360*64)

         # draw name
         layout = area.create_pango_layout(sys_name)
         area.window.draw_layout(gc, dx+r/2+2, dy-r/2, layout)

      # draw the frame at the end
      area.window.draw_rectangle(sys_gc, False, 0,0, ww-1,wh-1)


   def __jump_add(self, wgt=None, event=None):
      if self.space_sel in self.systems.keys() and self.cur_system in self.systems.keys():
         self.systems[self.cur_system]["jumps"].append(self.space_sel)
         self.systems[self.space_sel]["jumps"].append(self.cur_system)
         data.uniq(self.systems[self.cur_system]["jumps"])
         data.uniq(self.systems[self.space_sel]["jumps"])
        
         self.__supdate()
         self.__space_draw()

   def __jump_rm(self, wgt=None, event=None):
      if self.space_sel in self.systems.keys() and self.cur_system in self.systems.keys():
         i = 0
         for e in self.systems[self.cur_system]["jumps"]:
            if e == self.space_sel:
               self.systems[self.cur_system]["jumps"].pop(i)
            i = i + 1
         i = 0
         for e in self.systems[self.space_sel]["jumps"]:
            if e == self.cur_system:
               self.systems[self.space_sel]["jumps"].pop(i)
            i = i + 1
         self.__supdate()
         self.__space_draw()


   def __snew(self, wgt=None, event=None):
      name = "new system"
      gen = { "asteroids":0, "interference":0, "stars":100 }
      pos = { "x":0,"y":0 }
      new_ssys = { "general":gen, "pos":pos, "jumps":[], "fleets":{}, "planets":[] }
      self.systems[name] = new_ssys
      self.__create_treSystems()
      self.__selSys(name)


   def __pnew(self, wgt=None, event=None):
      name = "new planet"
      gfx = { "space":"none.png" }
      gen = { "class":"A", "services":0 }
      pos = { "x":0,"y":0 }
      new_planet = { "GFX":gfx, "general":gen, "pos":pos }
      self.planets[name] = new_planet
      self.__create_trePlanets()
      self.__selPlanet(name)


   def __selSys(self, system):
      i = 0
      tree = self.__swidget("treSystems")
      for row in tree.get_model():
         if row[0] == system:
            tree.set_cursor(i)
            self.__supdate()
            break
         i = i+1

   def __selPlanet(self, planet):
      i = 0
      tree = self.__pwidget("trePlanets")
      for row in tree.get_model():
         if row[0] == planet:
            tree.set_cursor(i)
            self.__pupdate()
            break
         i = i+1

   def debug(self):
      print "SYSTEMS LOADED:"
      print
      for name, sys in self.systems.items():
         print "SYSTEM: %s" % name
         print sys
      print
      print
      print "PLANETS LOADED:"
      print self.planets


