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
ssys["Scout"] = function ()
   return "Unicorp PT-16 Core System"
end
ssys["Interceptor"] = function ()
   return "Unicorp PT-16 Core System"
end
ssys["Fighter"] = function ()
   return "Unicorp PT-68 Core System"
end
ssys["Bomber"] = function ()
   return "Unicorp PT-68 Core System"
end
ssys["Corvette"] = function ()
   return "Unicorp PT-200 Core System"
end
ssys["Destroyer"] = function ()
   return "Unicorp PT-310 Core System"
end
ssys["Cruiser"] = function ()
   return "Unicorp PT-500 Core System"
end
ssys["Battleship"] = function ()
   return "Unicorp PT-2200 Core System"
end
ssys["Carrier"] = function ()
   return "Unicorp PT-2200 Core System"
end
-- Civilian
ssys["Yacht"] = function ()
   return "Unicorp PT-16 Core System"
end
ssys["Courier"] = function ()
   return "Unicorp PT-68 Core System"
end
ssys["Freighter"] = function ()
   return "Unicorp PT-310 Core System"
end
ssys["Armoured Transport"] = ssys["Freighter"]
ssys["Bulk Freighter"] = function ()
   return "Unicorp PT-2200 Core System"
end

-- STANDARD HULLS
local shul = {}
shul["Scout"] = function ()
   return "Unicorp D-2 Light Plating"
end
shul["Interceptor"] = function ()
   return "Unicorp D-2 Light Plating"
end
shul["Fighter"] = function ()
   return "Unicorp D-4 Light Plating"
end
shul["Bomber"] = function ()
   return "Unicorp D-4 Light Plating"
end
shul["Corvette"] = function ()
   return "Unicorp D-12 Medium Plating"
end
shul["Destroyer"] = function ()
   return "Unicorp D-24 Medium Plating"
end
shul["Cruiser"] = function ()
   return "Unicorp D-48 Heavy Plating"
end
shul["Battleship"] = function ()
   return "Unicorp D-68 Heavy Plating"
end
shul["Carrier"] = function ()
   return "Unicorp D-68 Heavy Plating"
end
-- Civilian
shul["Yacht"] = function ()
   return choose_one{ "Unicorp D-2 Light Plating", "S&K Small Cargo Hull" }
end
shul["Courier"] = function ()
   return choose_one{ "Unicorp D-4 Light Plating", "S&K Small Cargo Hull" }
end
shul["Freighter"] = function ()
   return choose_one{ "Unicorp D-24 Medium Plating", "S&K Medium Cargo Hull" }
end
shul["Armoured Transport"] = shul["Freighter"]
shul["Bulk Freighter"] = function ()
   return "Unicorp D-48 Heavy Plating"
end

-- STANDARD ENGINES
local seng = {}
seng["Scout"] = function ()
   return "Nexus Dart 160 Engine"
end
seng["Interceptor"] = function ()
   return "Nexus Dart 160 Engine"
end
seng["Fighter"] = function ()
   return "Unicorp Hawk 360 Engine"
end
seng["Bomber"] = function ()
   return "Unicorp Hawk 360 Engine"
end
seng["Corvette"] = function ()
   return "Nexus Arrow 700 Engine"
end
seng["Destroyer"] = function ()
   return "Unicorp Falcon 1400 Engine"
end
seng["Cruiser"] = function ()
   return "Nexus Bolt 3000 Engine"
end
seng["Battleship"] = function ()
   return "Unicorp Eagle 6500 Engine"
end
seng["Carrier"] = function ()
   return "Unicorp Eagle 6500 Engine"
end
seng["Yacht"] = function ()
   return "Nexus Dart 160 Engine"
end
seng["Courier"] = function ()
   return "Unicorp Hawk 360 Engine"
end
seng["Freighter"] = function ()
   return choose_one{ "Unicorp Falcon 1400 Engine", "Melendez Buffalo XL Engine" }
end
seng["Armoured Transport"] = seng["Freighter"]
seng["Bulk Freighter"] = function ()
   return "Nexus Bolt 3000 Engine"
end

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
esys["Scout"] = function ()
   return "Milspec Orion 2301 Core System"
end
esys["Interceptor"] = function ()
   return choose_one{ "Milspec Orion 2301 Core System", "Milspec Thalos 2202 Core System" }
end
esys["Fighter"] = function ()
   return choose_one{ "Milspec Orion 3701 Core System", "Milspec Thalos 3602 Core System" }
end
esys["Bomber"] = function ()
   return "Milspec Orion 3701 Core System"
end
esys["Corvette"] = function ()
   return choose_one{ "Milspec Orion 4801 Core System", "Milspec Thalos 4702 Core System" }
end
esys["Destroyer"] = function ()
   return choose_one{ "Milspec Orion 5501 Core System", "Milspec Thalos 5402 Core System" }
end
esys["Cruiser"] = function ()
   return choose_one{ "Milspec Orion 9901 Core System", "Milspec Thalos 9802 Core System" }
end
esys["Battleship"] = function ()
   return choose_one{ "Milspec Orion 8601 Core System", "Milspec Thalos 8502 Core System" }
end
esys["Carrier"] = function ()
   return choose_one{ "Milspec Orion 9901 Core System", "Milspec Thalos 9802 Core System" }
end
-- Civilian
esys["Yacht"] = ssys["Yacht"]
esys["Courier"] = ssys["Courier"]
esys["Freighter"] = ssys["Freighter"]
esys["Armoured Transport"] = function ()
   return "Milspec Orion 5501 Core System"
end
esys["Bulk Freighter"] = function ()
   return "Milspec Orion 8601 Core System"
end

-- ELITE HULLS
local ehul = {}
ehul["Scout"] = function ()
   return choose_one{ "Nexus Light Stealth Plating", "S&K Ultralight Combat Plating" }
end
ehul["Interceptor"] = function ()
   return choose_one{ "Nexus Light Stealth Plating", "S&K Ultralight Combat Plating" }
end
ehul["Fighter"] = function ()
   return choose_one{ "Nexus Light Stealth Plating", "S&K Light Combat Plating" }
end
ehul["Bomber"] = function ()
   return choose_one{ "Nexus Light Stealth Plating", "S&K Light Combat Plating" }
end
ehul["Corvette"] = function ()
   return choose_one{ "Nexus Medium Stealth Plating", "S&K Medium Combat Plating" }
end
ehul["Destroyer"] = function ()
   return "S&K Medium-Heavy Combat Plating"
end
ehul["Cruiser"] = function ()
   return choose_one{ "Unicorp D-48 Heavy Plating", "Unicorp D-68 Heavy Plating" }
end
ehul["Battleship"] = function ()
   return "S&K Superheavy Combat Plating"
end
ehul["Carrier"] = function ()
   return "S&K Superheavy Combat Plating"
end
-- Civilian
ehul["Yacht"] = shul["Yacht"]
ehul["Courier"] = shul["Courier"]
ehul["Freighter"] = shul["Freighter"]
ehul["Armoured Transport"] = function ()
   return choose_one{ "Unicorp D-24 Medium Plating", "Nexus Medium Stealth Plating" }
end
ehul["Bulk Freighter"] = function ()
   return "S&K Large Cargo Hull"
end

-- ELITE ENGINES
local eeng = {}
eeng["Scout"] = function ()
   return "Tricon Zephyr Engine"
end
eeng["Interceptor"] = function ()
   return "Tricon Zephyr Engine"
end
eeng["Fighter"] = function ()
   return "Tricon Zephyr II Engine"
end
eeng["Bomber"] = function ()
   return "Tricon Zephyr II Engine"
end
eeng["Corvette"] = function ()
   return "Tricon Cyclone Engine"
end
eeng["Destroyer"] = function ()
   return choose_one{ "Tricon Cyclone II Engine", "Melendez Buffalo XL Engine" }
end
eeng["Cruiser"] = function ()
   return "Tricon Typhoon Engine"
end
eeng["Battleship"] = function ()
   return choose_one{ "Tricon Typhoon II Engine", "Melendez Mammoth XL Engine" }
end
eeng["Carrier"] = function ()
   return "Melendez Mammoth XL Engine"
end
eeng["Yacht"] = seng["Yacht"]
eeng["Courier"] = seng["Courier"]
eeng["Freighter"] = seng["Freighter"]
eeng["Armoured Transport"] = seng["Armoured Transport"]
eeng["Bulk Freighter"] = function ()
   return "Melendez Mammoth XL Engine"
end

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
cores.standard.engines["Kestrel"] = function ()
   return choose_one{ "Unicorp Eagle 6500 Engine", "Krain Remige Engine" }
end
cores.standard.engines["Pirate Kestrel"] = cores.standard.engines["Kestrel"]
cores.standard.engines["Starbridge"] = function ()
   return choose_one{ "Unicorp Falcon 1400 Engine", "Krain Patagium Engine" }
end
cores.standard.engines["Pirate Starbridge"] = cores.standard.engines["Starbridge"]
cores.standard.systems["Proteron Hippocrates"] = function ()
   return "Milspec Thalos 5402 Core System"
end

-- Elite exceptions
cores.elite.engines["Kestrel"] = function ()
   return choose_one{ "Tricon Typhoon Engine", "Krain Remige Engine" }
end
cores.elite.engines["Pirate Kestrel"] = cores.elite.engines["Kestrel"]
cores.elite.engines["Starbridge"] = function ()
   return choose_one{ "Tricon Cyclone II Engine", "Krain Patagium Engine" }
end
cores.elite.engines["Pirate Starbridge"] = cores.elite.engines["Starbridge"]
cores.elite.systems["Gawain"] = function ()
   return choose_one{ "Unicorp PT-16 Core System", "Milspec Orion 2301 Core System" }
end
cores.elite.hulls["Gawain"] = function ()
   return choose_one{ "Unicorp D-2 Light Plating", "Nexus Light Stealth Plating" }
end
cores.elite.engines["Gawain"] = function ()
   return choose_one{ "Nexus Dart 160 Engine", "Tricon Zephyr Engine" }
end
cores.elite.systems["Proteron Hippocrates"] = function ()
   return "Milspec Thalos 5402 Core System"
end

--[[
cores.get( "Fighter", { all="elite" } )
cores.get( "Fighter", { all={"standard","elite"} )
cores.get( "Fighter", { systems="elite", hulls="standard", engines="elite" } )
--]]
function cores.get( p, params )
   if params == nil then
      return nil
   end
   local s = p:ship()
   if s:tags().bioship then
      return
   end
   local shipclass = params.class or s:class()
   local shipname = s:nameRaw()

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
      table.insert( c, co() )
   end
   if hulls then
      local ct = cores[ hulls ].hulls
      local co = ct[ shipname ] or ct[ shipclass ]
      table.insert( c, co() )
   end
   if engines then
      local ct = cores[ engines ].engines
      local co = ct[ shipname ] or ct[ shipclass ]
      table.insert( c, co() )
   end

   return c
end

return cores
