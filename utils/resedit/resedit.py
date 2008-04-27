#!/usr/bin/env python
"""
Copyright (C) 2008 by Edgar Simo Serra
bobbens@gmail.com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""

try:
   import gtk,gtk.glade
   import gobject
except:
   print "You do not have python gtk bindings, or you're missing glade libs"
   print "To use resedit you must install them"
   print "http://pygtk.org/ "
   raise SystemExit


import space, faction, fleet


# load the factions
factions = faction.Factions()
factions.loadFactions("../../dat/faction.xml")
factions.window()

# load the fleets
fleets = fleet.Fleets()
fleets.loadFleets("../../dat/fleet.xml")

# load the universe
universe = space.Space( factions.data(), fleets.data() )
universe.loadSystems("../../dat/ssys.xml")
universe.loadPlanets("../../dat/planet.xml")
universe.window()



gtk.main()



