local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end
local cores = {}
cores.default = {}
cores.elite = {}
cores.elite.systems = {
   ["Fighter"] = function( heavy )
      if heavy then
         return choose_one{ "Milspec Orion 3701 Core System", "Milspec Thalos 3602 Core System" }
      else
         return choose_one{ "Milspec Orion 2301 Core System", "Milspec Thalos 2202 Core System" }
      end
   end,
   ["Bomber"] = function( heavy )
      return choose_one{ "Milspec Orion 3701 Core System", "Milspec Thalos 3602 Core System" }
   end,
   ["Corvette"] = function( heavy )
      return "Milspec Orion 4801 Core System"
   end,
   ["Destroyer"] = function( heavy )
      return "Milspec Orion 5501 Core System"
   end,
   ["Cruiser"] = function( heavy )
      if heavy then
         return "Milspec Orion 8601 Core System"
      else
         return "Milspec Orion 9901 Core System"
      end
   end,
   ["Carrier"] = function( heavy )
      return "Milspec Orion 9901 Core System"
   end,
}
cores.elite.hulls = {
   ["Fighter"] = function( heavy )
      if heavy then
         return choose_one{ "Nexus Light Stealth Plating", "S&K Light Combat Plating" }
      else
         return choose_one{ "Nexus Light Stealth Plating", "S&K Ultralight Combat Plating" }
      end
   end,
   ["Bomber"] = function( heavy )
      return choose_one{ "Nexus Light Stealth Plating", "S&K Light Combat Plating" }
   end,
   ["Corvette"] = function( heavy )
      return choose_one{ "Nexus Medium Stealth Plating", "S&K Medium Combat Plating" }
   end,
   ["Destroyer"] = function( heavy )
      return "S&K Medium-Heavy Combat Plating"
   end,
   ["Cruiser"] = function( heavy )
      if heavy then
         return "S&K Superheavy Combat Plating"
      else
         return choose_one{ "Unicorp D-48 Heavy Plating", "Unicorp D-68 Heavy Plating" }
      end
   end,
   ["Carrier"] = function( heavy )
      return "Milspec Orion 9901 Core System"
   end,
}
cores.elite.engines = {
   ["Fighter"] = function( heavy )
      if heavy then
         return "Tricon Zephyr Engine"
      else
         return "Tricon Zephyr II Engine"
      end
   end,
   ["Bomber"] = function( heavy )
      return "Tricon Zephyr Engine"
   end,
   ["Corvette"] = function( heavy )
      return "Tricon Cyclone Engine"
   end,
   ["Destroyer"] = function( heavy )
      return "Tricon Cyclone II Engine"
   end,
   ["Cruiser"] = function( heavy )
      if heavy then
         return "Tricon Typhoon II Engine"
      else
         return "Tricon Typhoon Engine"
      end
   end,
   ["Carrier"] = function( heavy )
      return "Melendez Mammoth XL Engine"
   end,
}
local ces = cores.elite.systems
local ceh = cores.elite.hull
local cee = cores.elite.engines
local function choose_set( c, heavy )
   heavy = heavy or (rnd.rnd() > 0.5)
   return { cee[c](heavy), ceh[c](heavy), ces[c](heavy) }
end
cores.elite.set = {
   ["Fighter"]  = function( heavy ) return choose_set( "Fighter",  heavy ) end,
   ["Bomber"]   = function( heavy ) return choose_set( "Bomber",   heavy ) end,
   ["Corvette"] = function( heavy ) return choose_set( "Corvette", heavy ) end,
   ["Destroyer"]= function( heavy ) return choose_set( "Destroyer",heavy ) end,
   ["Cruiser"]  = function( heavy ) return choose_set( "Cruiser",  heavy ) end,
   ["Carrier"]  = function( heavy ) return choose_set( "Carrier",  heavy ) end,
}

return cores
