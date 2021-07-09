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
ssys["Fighter"] = function( heavy )
   if heavy then
      return "Unicorp PT-68 Core System"
   else
      return "Unicorp PT-16 Core System"
   end
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
   if heavy then
      return "Unicorp PT-2200 Core System"
   else
      return "Unicorp PT-500 Core System"
   end
end
ssys["Carrier"] = function( heavy )
   return "Unicorp PT-2200 Core System"
end
-- Civilian
ssys["Yacht"] = function( heavy )
   return "Unicorp PT-16 Core System"
end
ssys["Luxury Yacht"] = ssys["Yacht"]
ssys["Courier"] = function( heavy )
   return "Unicorp PT-68 Core System"
end
ssys["Freighter"] = function( heavy )
   return "Unicorp PT-310 Core System"
end
ssys["Armoured Transport"] = ssys["Freighter"]
-- Robotic
ssys["Drone"] = ssys["Fighter"]
ssys["Heavy Drone"] = ssys["Corvette"]

-- STANDARD HULLS
local shul = {}
shul["Scout"] = function( heavy )
   return "Unicorp D-2 Light Plating"
end
shul["Fighter"] = function( heavy )
   if heavy then
      return "Unicorp D-4 Light Plating"
   else
      return "Unicorp D-2 Light Plating"
   end
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
   if heavy then
      return "Unicorp D-68 Heavy Plating"
   else
      return "Unicorp D-48 Heavy Plating"
   end
end
shul["Carrier"] = function( heavy )
   return "Unicorp D-68 Heavy Plating"
end
-- Civilian
shul["Yacht"] = function( heavy )
   return choose_one{ "Unicorp D-2 Light Plating", "S&K Small Cargo Hull" }
end
shul["Luxury Yacht"] = shul["Yacht"]
shul["Courier"] = function( heavy )
   return choose_one{ "Unicorp D-4 Light Plating", "S&K Small Cargo Hull" }
end
shul["Freighter"] = function( heavy )
   return choose_one{ "Unicorp D-24 Medium Plating", "S&K Medium Cargo Hull" }
end
shul["Armoured Transport"] = shul["Freighter"]
-- Robotic
shul["Drone"] = shul["Fighter"]
shul["Heavy Drone"] = shul["Corvette"]

-- STANDARD ENGINES
local seng = {}
seng["Scout"] = function( heavy )
   return "Nexus Dart 150 Engine"
end
seng["Fighter"] = function( heavy )
   if heavy then
      return "Unicorp Hawk 350 Engine"
   else
      return "Nexus Dart 150 Engine"
   end
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
   if heavy then
      return "Nexus Bolt 4500 Engine"
   else
      return "Unicorp Eagle 7000 Engine"
   end
end
seng["Carrier"] = function( heavy )
   return "Unicorp Eagle 7000 Engine"
end
seng["Yacht"] = function( heavy )
   return "Nexus Dart 150 Engine"
end
seng["Luxury Yacht"] = seng["Yacht"]
seng["Courier"] = function( heavy )
   return "Unicorp Hawk 350 Engine"
end
seng["Freighter"] = function( heavy )
   return choose_one{ "Unicorp Falcon 1300 Engine", "Melendez Buffalo XL Engine" }
end
seng["Armoured Transport"] = seng["Freighter"]
-- Robotic
seng["Drone"] = seng["Fighter"]
seng["Heavy Drone"] = seng["Corvette"]

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
esys["Fighter"] = function( heavy )
   if heavy then
      return choose_one{ "Milspec Orion 3701 Core System", "Milspec Thalos 3602 Core System" }
   else
      return choose_one{ "Milspec Orion 2301 Core System", "Milspec Thalos 2202 Core System" }
   end
end
esys["Bomber"] = function( heavy )
   return choose_one{ "Milspec Orion 3701 Core System", "Milspec Thalos 3602 Core System" }
end
esys["Corvette"] = function( heavy )
   return choose_one{ "Milspec Orion 4801 Core System", "Milspec Thalos 4702 Core System" }
end
esys["Destroyer"] = function( heavy )
   return choose_one{ "Milspec Orion 5501 Core System", "Milspec Thalos 5402 Core System" }
end
esys["Cruiser"] = function( heavy )
   if heavy then
      return choose_one{ "Milspec Orion 8601 Core System", "Milspec Thalos 8502 Core System" }
   else
      return choose_one{ "Milspec Orion 9901 Core System", "Milspec Thalos 9802 Core System" }
   end
end
esys["Carrier"] = function( heavy )
   return choose_one{ "Milspec Orion 9901 Core System", "Milspec Thalos 9802 Core System" }
end
-- Civilian
esys["Yacht"] = ssys["Yacht"]
esys["Luxury Yacht"] = function( heavy )
   return choose_one{ "Unicorp PT-16 Core System", "Milspec Orion 2301 Core System" }
end
esys["Courier"] = ssys["Courier"]
esys["Freighter"] = ssys["Freighter"]
esys["Armoured Transport"] = function( heavy )
   return "Milspec Orion 5501 Core System"
end
-- Robotic
esys["Drone"] = esys["Fighter"]
esys["Heavy Drone"] = esys["Corvette"]

-- ELITE HULLS
local ehul = {}
ehul["Scout"] = function( heavy )
   return choose_one{ "Nexus Light Stealth Plating", "S&K Ultralight Combat Plating" }
end
ehul["Fighter"] = function( heavy )
   if heavy then
      return choose_one{ "Nexus Light Stealth Plating", "S&K Light Combat Plating" }
   else
      return choose_one{ "Nexus Light Stealth Plating", "S&K Ultralight Combat Plating" }
   end
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
   if heavy then
      return "S&K Superheavy Combat Plating"
   else
      return choose_one{ "Unicorp D-48 Heavy Plating", "Unicorp D-68 Heavy Plating" }
   end
end
ehul["Carrier"] = function( heavy )
   return "S&K Superheavy Combat Plating"
end
-- Civilian
ehul["Yacht"] = shul["Yacht"]
ehul["Luxury Yacht"] = function( heavy )
   return choose_one{ "Unicorp D-2 Light Plating", "Nexus Light Stealth Plating" }
end
ehul["Courier"] = shul["Courier"]
ehul["Freighter"] = shul["Freighter"]
ehul["Armoured Transport"] = function( heavy )
   return choose_one{ "Unicorp D-24 Medium Plating", "Nexus Medium Stealth Plating" }
end
-- Robotic
ehul["Drone"] = ehul["Fighter"]
ehul["Heavy Drone"] = ehul["Corvette"]

-- ELITE ENGINES
local eeng = {}
eeng["Scout"] = function( heavy )
   return "Tricon Zephyr Engine"
end
eeng["Fighter"] = function( heavy )
   if heavy then
      return "Tricon Zephyr II Engine"
   else
      return "Tricon Zephyr Engine"
   end
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
   if heavy then
      return choose_one{ "Tricon Typhoon II Engine", "Melendez Mammoth XL Engine" }
   else
      return "Tricon Typhoon Engine"
   end
end
eeng["Carrier"] = function( heavy )
   return "Melendez Mammoth XL Engine"
end
eeng["Yacht"] = seng["Yacht"]
eeng["Luxury Yacht"] = function( heavy )
   return choose_one{ "Nexus Dart 150 Engine", "Tricon Zephyr Engine" }
end
eeng["Courier"] = seng["Courier"]
eeng["Freighter"] = seng["Freighter"]
eeng["Armoured Transport"] = seng["Armoured Transport"]
-- Robotic
eeng["Drone"] = eeng["Fighter"]
eeng["Heavy Drone"] = eeng["Corvette"]

-- ELITE SETS
cores.elite = {
   systems = esys,
   hulls   = ehul,
   engines = eeng,
}


--[[
   SHIP-BASED EXCEPTIONS
--]]
local heavy_exception = {
   ["Lancelot"] = true,
   ["Goddard"] = true,
}
-- Normal exceptions
cores.standard.engines["Kestrel"] = function( heavy )
   if heavy then
      return seng["Cruiser"]( heavy )
   end
   return choose_one{ "Unicorp Eagle 7000 Engine", "Krain Remige Engine" }
end
cores.standard.engines["Pirate Kestrel"] = cores.standard.engines["Kestrel"]

-- Elite exceptions
cores.standard.engines["Kestrel"] = function( heavy )
   if heavy then
      return eeng["Cruiser"]( heavy )
   end
   return choose_one{ "Tricon Typhoon Engine", "Krain Remige Engine" }
end
cores.elite.engines["Pirate Kestrel"] = cores.elite.engines["Kestrel"]

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
      heavy = heavy_exception[shipname] or (rnd.rnd() > 0.5)
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
