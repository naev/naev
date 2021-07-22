local cores = {}

--[[
-- Helper functions
--]]
local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end
local function choose_one_table( t )
   if type(t) ~= "table" then
      return t
   end
   return choose_one( t )
end


--[[
      STANDARD CORES
--]]
-- STANDARD SYSTEMS
local ssys = {}
ssys["Scout"] = function( heavy )
   return "Unicorp PT-16 Core System"
end
ssys["Interceptor"] = function( heavy )
   return "Unicorp PT-16 Core System"
end
ssys["Fighter"] = function( heavy )
   return "Unicorp PT-68 Core System"
end
ssys["Bomber"] = function( heavy )
   return "Unicorp PT-68 Core System"
end
ssys["Corvette"] = function( heavy )
   return "Unicorp PT-200 Core System"
end
ssys["Destroyer"] = function( heavy )
   return "Unicorp PT-310 Core System"
end
ssys["Cruiser"] = function( heavy )
   return "Unicorp PT-500 Core System"
end
ssys["Battleship"] = function( heavy )
   return "Unicorp PT-2200 Core System"
end
ssys["Carrier"] = function( heavy )
   return "Unicorp PT-2200 Core System"
end
-- Civilian
ssys["Yacht"] = function( heavy )
   return "Unicorp PT-16 Core System"
end
ssys["Courier"] = function( heavy )
   return "Unicorp PT-68 Core System"
end
ssys["Freighter"] = function( heavy )
   return "Unicorp PT-310 Core System"
end
ssys["Armoured Transport"] = ssys["Freighter"]
ssys["Bulk Freighter"] = function( heavy )
   return "Unicorp PT-500 Core System"
end

-- STANDARD HULLS
local shul = {}
shul["Scout"] = function( heavy )
   return "Unicorp D-2 Light Plating"
end
shul["Interceptor"] = function( heavy )
   return "Unicorp D-2 Light Plating"
end
shul["Fighter"] = function( heavy )
   return "Unicorp D-4 Light Plating"
end
shul["Bomber"] = function( heavy )
   return "Unicorp D-4 Light Plating"
end
shul["Corvette"] = function( heavy )
   return "Unicorp D-12 Medium Plating"
end
shul["Destroyer"] = function( heavy )
   return "Unicorp D-24 Medium Plating"
end
shul["Cruiser"] = function( heavy )
   return "Unicorp D-48 Heavy Plating"
end
shul["Battleship"] = function( heavy )
   return "Unicorp D-68 Heavy Plating"
end
shul["Carrier"] = function( heavy )
   return "Unicorp D-68 Heavy Plating"
end
-- Civilian
shul["Yacht"] = function( heavy )
   return choose_one{ "Unicorp D-2 Light Plating", "S&K Small Cargo Hull" }
end
shul["Courier"] = function( heavy )
   return choose_one{ "Unicorp D-4 Light Plating", "S&K Small Cargo Hull" }
end
shul["Freighter"] = function( heavy )
   return choose_one{ "Unicorp D-24 Medium Plating", "S&K Medium Cargo Hull" }
end
shul["Armoured Transport"] = shul["Freighter"]

-- STANDARD ENGINES
local seng = {}
seng["Scout"] = function( heavy )
   return "Nexus Dart 150 Engine"
end
seng["Interceptor"] = function( heavy )
   return "Nexus Dart 150 Engine"
end
seng["Fighter"] = function( heavy )
   return "Unicorp Hawk 350 Engine"
end
seng["Bomber"] = function( heavy )
   return "Unicorp Hawk 350 Engine"
end
seng["Corvette"] = function( heavy )
   return "Nexus Arrow 700 Engine"
end
seng["Destroyer"] = function( heavy )
   return "Unicorp Falcon 1300 Engine"
end
seng["Cruiser"] = function( heavy )
   return "Unicorp Eagle 7000 Engine"
end
seng["Battleship"] = function( heavy )
   return "Nexus Bolt 4500 Engine"
end
seng["Carrier"] = function( heavy )
   return "Unicorp Eagle 7000 Engine"
end
seng["Yacht"] = function( heavy )
   return "Nexus Dart 150 Engine"
end
seng["Courier"] = function( heavy )
   return "Unicorp Hawk 350 Engine"
end
seng["Freighter"] = function( heavy )
   return choose_one{ "Unicorp Falcon 1300 Engine", "Melendez Buffalo XL Engine" }
end
seng["Armoured Transport"] = seng["Freighter"]

-- STANDARD SETS
cores.standard = {
   systems = ssys,
   hulls   = shul,
   engines = seng,
}


--[[
      ELITE CORES
--]]
-- ELITE SYSTEMS
local esys = {}
esys["Scout"] = function( heavy )
   return "Milspec Orion 2301 Core System"
end
esys["Interceptor"] = function( heavy )
   return choose_one{ "Milspec Orion 2301 Core System", "Milspec Thalos 2202 Core System" }
end
esys["Fighter"] = function( heavy )
   return choose_one{ "Milspec Orion 3701 Core System", "Milspec Thalos 3602 Core System" }
end
esys["Bomber"] = function( heavy )
   return "Milspec Orion 3701 Core System"
end
esys["Corvette"] = function( heavy )
   return choose_one{ "Milspec Orion 4801 Core System", "Milspec Thalos 4702 Core System" }
end
esys["Destroyer"] = function( heavy )
   return choose_one{ "Milspec Orion 5501 Core System", "Milspec Thalos 5402 Core System" }
end
esys["Cruiser"] = function( heavy )
   return choose_one{ "Milspec Orion 9901 Core System", "Milspec Thalos 9802 Core System" }
end
esys["Battleship"] = function( heavy )
   return choose_one{ "Milspec Orion 8601 Core System", "Milspec Thalos 8502 Core System" }
end
esys["Carrier"] = function( heavy )
   return choose_one{ "Milspec Orion 9901 Core System", "Milspec Thalos 9802 Core System" }
end
-- Civilian
esys["Yacht"] = ssys["Yacht"]
esys["Courier"] = ssys["Courier"]
esys["Freighter"] = ssys["Freighter"]
esys["Armoured Transport"] = function( heavy )
   return "Milspec Orion 5501 Core System"
end
esys["Bulk Freighter"] = ssys["Bulk Freighter"]

-- ELITE HULLS
local ehul = {}
ehul["Scout"] = function( heavy )
   return choose_one{ "Nexus Light Stealth Plating", "S&K Ultralight Combat Plating" }
end
ehul["Interceptor"] = function( heavy )
   return choose_one{ "Nexus Light Stealth Plating", "S&K Ultralight Combat Plating" }
end
ehul["Fighter"] = function( heavy )
   return choose_one{ "Nexus Light Stealth Plating", "S&K Light Combat Plating" }
end
ehul["Bomber"] = function( heavy )
   return choose_one{ "Nexus Light Stealth Plating", "S&K Light Combat Plating" }
end
ehul["Corvette"] = function( heavy )
   return choose_one{ "Nexus Medium Stealth Plating", "S&K Medium Combat Plating" }
end
ehul["Destroyer"] = function( heavy )
   return "S&K Medium-Heavy Combat Plating"
end
ehul["Cruiser"] = function( heavy )
   return choose_one{ "Unicorp D-48 Heavy Plating", "Unicorp D-68 Heavy Plating" }
end
ehul["Battleship"] = function( heavy )
   return "S&K Superheavy Combat Plating"
end
ehul["Carrier"] = function( heavy )
   return "S&K Superheavy Combat Plating"
end
-- Civilian
ehul["Yacht"] = shul["Yacht"]
ehul["Courier"] = shul["Courier"]
ehul["Freighter"] = shul["Freighter"]
ehul["Armoured Transport"] = function( heavy )
   return choose_one{ "Unicorp D-24 Medium Plating", "Nexus Medium Stealth Plating" }
end
ehul["Bulk Freighter"] = shul["Bulk Freighter"]

-- ELITE ENGINES
local eeng = {}
eeng["Scout"] = function( heavy )
   return "Tricon Zephyr Engine"
end
eeng["Interceptor"] = function( heavy )
   return "Tricon Zephyr Engine"
end
eeng["Fighter"] = function( heavy )
   return "Tricon Zephyr II Engine"
end
eeng["Bomber"] = function( heavy )
   return "Tricon Zephyr II Engine"
end
eeng["Corvette"] = function( heavy )
   return "Tricon Cyclone Engine"
end
eeng["Destroyer"] = function( heavy )
   return choose_one{ "Tricon Cyclone II Engine", "Melendez Buffalo XL Engine" }
end
eeng["Cruiser"] = function( heavy )
   return "Tricon Typhoon Engine"
end
eeng["Battleship"] = function( heavy )
   return choose_one{ "Tricon Typhoon II Engine", "Melendez Mammoth XL Engine" }
end
eeng["Carrier"] = function( heavy )
   return "Melendez Mammoth XL Engine"
end
eeng["Yacht"] = seng["Yacht"]
eeng["Courier"] = seng["Courier"]
eeng["Freighter"] = seng["Freighter"]
eeng["Armoured Transport"] = seng["Armoured Transport"]
eeng["Bulk Freighter"] = seng["Bulk Freighter"]

-- ELITE SETS
cores.elite = {
   systems = esys,
   hulls   = ehul,
   engines = eeng,
}


--[[
   SHIP-BASED EXCEPTIONS
--]]
-- Normal exceptions
cores.standard.engines["Kestrel"] = function( heavy )
   return choose_one{ "Unicorp Eagle 7000 Engine", "Krain Remige Engine" }
end
cores.standard.engines["Pirate Kestrel"] = cores.standard.engines["Kestrel"]

-- Elite exceptions
cores.standard.engines["Kestrel"] = function( heavy )
   return choose_one{ "Tricon Typhoon Engine", "Krain Remige Engine" }
end
cores.elite.engines["Pirate Kestrel"] = cores.elite.engines["Kestrel"]
cores.elite.systems["Gawain"] = function( heavy )
   return choose_one{ "Unicorp PT-16 Core System", "Milspec Orion 2301 Core System" }
end
cores.elite.hulls["Gawain"] = function( heavy )
   return choose_one{ "Unicorp D-2 Light Plating", "Nexus Light Stealth Plating" }
end
cores.elite.engines["Gawain"] = function( heavy )
   return choose_one{ "Nexus Dart 150 Engine", "Tricon Zephyr Engine" }
end

-- Load Soromid Exceptions
require "equipopt.cores_soromid"( cores )

--[[
cores.get( "Fighter", { all="elite" } )
cores.get( "Fighter", { all={"standard","elite"}, heavy=true } )
cores.get( "Fighter", { systems="elite", hulls="standard", engines="elite" } )
--]]
function cores.get( p, params )
   if params == nil then
      return nil
   end
   local s = p:ship()
   local shipclass = s:class()
   local shipname = s:nameRaw()

   -- Check out if we have to do heavy
   local heavy
   if params.heavy == nil then
      heavy = rnd.rnd() > 0.5
   else
      heavy = params.heavy
   end

   -- Find out what type to use for each slot
   local systems, hulls, engines
   if params.all then
      local all = choose_one_table( params.all )
      systems  = all
      hulls    = all
      engines  = all
   else
      systems  = choose_one_table( params.systems )
      hulls    = choose_one_table( params.hulls )
      engines  = choose_one_table( params.engines )
   end

   -- Get the cores if applicable
   local c = {}
   if systems then
      local ct = cores[ systems ].systems
      local co = ct[ shipname ] or ct[ shipclass ]
      table.insert( c, co( heavy ) )
   end
   if hulls then
      local ct = cores[ hulls ].hulls
      local co = ct[ shipname ] or ct[ shipclass ]
      table.insert( c, co( heavy ) )
   end
   if engines then
      local ct = cores[ engines ].engines
      local co = ct[ shipname ] or ct[ shipclass ]
      table.insert( c, co( heavy ) )
   end

   return c
end

return cores
