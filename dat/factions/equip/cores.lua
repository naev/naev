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
      NORMAL CORES
--]]
-- NORMAL SYSTEMS
local nsys = {}
nsys["Fighter"] = function( heavy )
   if heavy then
      return "Unicorp PT-68 Core System"
   else
      return "Unicorp PT-16 Core System"
   end
end
nsys["Bomber"] = function( heavy )
   return "Unicorp PT-68 Core System"
end
nsys["Corvette"] = function( heavy )
   return "Unicorp PT-200 Core System"
end
nsys["Destroyer"] = function( heavy )
   return "Unicorp PT-310 Core System"
end
nsys["Cruiser"] = function( heavy )
   if heavy then
      return "Unicorp PT-2200 Core System"
   else
      return "Unicorp PT-500 Core System"
   end
end
nsys["Carrier"] = function( heavy )
   return "Unicorp PT-2200 Core System"
end
-- Civilian
nsys["Yacht"] = function( heavy )
   return "Unicorp PT-16 Core System"
end
nsys["Luxury Yacht"] = nsys["Yacht"]
nsys["Courier"] = function( heavy )
   return "Unicorp PT-68 Core System"
end
nsys["Freighter"] = function( heavy )
   return "Unicorp PT-310 Core System"
end
nsys["Armoured Transport"] = nsys["Freighter"]

-- NORMAL HULLS
local nhul = {}
nhul["Fighter"] = function( heavy )
   if heavy then
      return "Unicorp D-4 Light Plating"
   else
      return "Unicorp D-2 Light Plating"
   end
end
nhul["Bomber"] = function( heavy )
   return "Unicorp D-4 Light Plating"
end
nhul["Corvette"] = function( heavy )
   return "Unicorp D-12 Medium Plating"
end
nhul["Destroyer"] = function( heavy )
   return "Unicorp D-24 Medium Plating"
end
nhul["Cruiser"] = function( heavy )
   if heavy then
      return "Unicorp D-68 Heavy Plating"
   else
      return "Unicorp D-48 Heavy Plating"
   end
end
nhul["Carrier"] = function( heavy )
   return "Unicorp D-68 Heavy Plating"
end
-- Civilian
nhul["Yacht"] = function( heavy )
   return choose_one{ "Unicorp D-2 Light Plating", "S&K Small Cargo Hull" }
end
nhul["Luxury Yacht"] = nhul["Yacht"]
nhul["Courier"] = function( heavy )
   return choose_one{ "Unicorp D-4 Light Plating", "S&K Small Cargo Hull" }
end
nhul["Freighter"] = function( heavy )
   return choose_one{ "Unicorp D-24 Medium Plating", "S&K Medium Cargo Hull" }
end
nhul["Armoured Transport"] = nhul["Freighter"]

-- NORMAL ENGINES
local neng = {}
neng["Fighter"] = function( heavy )
   if heavy then
      return "Unicorp Hawk 350 Engine"
   else
      return "Nexus Dart 150 Engine"
   end
end
neng["Bomber"] = function( heavy )
   return "Unicorp Hawk 350 Engine"
end
neng["Corvette"] = function( heavy )
   return "Nexus Arrow 700 Engine"
end
neng["Destroyer"] = function( heavy )
   return "Unicorp Falcon 1300 Engine"
end
neng["Cruiser"] = function( heavy )
   if heavy then
      return "Nexus Bolt 4500 Engine"
   else
      return "Unicorp Eagle 7000 Engine"
   end
end
neng["Carrier"] = function( heavy )
   return "Unicorp Eagle 7000 Engine"
end
neng["Yacht"] = function( heavy )
   return "Nexus Dart 150 Engine"
end
neng["Luxury Yacht"] = neng["Yacht"]
neng["Courier"] = function( heavy )
   return "Unicorp Hawk 350 Engine"
end
neng["Freighter"] = function( heavy )
   return choose_one{ "Unicorp Falcon 1300 Engine", "Melendez Buffalo XL Engine" }
end
neng["Armoured Transport"] = neng["Freighter"]

-- NORMAL SETS
cores.normal = {
   systems = nsys,
   hulls   = nhul,
   engines = neng,
}


--[[
      ELITE CORES
--]]
-- ELITE SYSTEMS
local esys = {}
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
esys["Yacht"] = nsys["Yacht"]
esys["Luxury Yacht"] = function( heavy )
   return choose_one{ "Unicorp PT-16 Core System", "Milspec Orion 2301 Core System" }
end
esys["Courier"] = nsys["Courier"]
esys["Freighter"] = nsys["Freighter"]
esys["Armoured Transport"] = function( heavy )
   return "Milspec Orion 5501 Core System"
end

-- ELITE HULLS
local ehul = {}
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
ehul["Yacht"] = nhul["Yacht"]
ehul["Luxury Yacht"] = function( heavy )
   return choose_one{ "Unicorp D-2 Light Plating", "Nexus Light Stealth Plating" }
end
ehul["Courier"] = nhul["Courier"]
ehul["Freighter"] = nhul["Freighter"]
ehul["Armoured Transport"] = function( heavy )
   return choose_one{ "Unicorp D-24 Medium Plating", "Nexus Medium Stealth Plating" }
end

-- ELITE ENGINES
local eeng = {}
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
eeng["Yacht"] = neng["Yacht"]
eeng["Luxury Yacht"] = function( heavy )
   return choose_one{ "Nexus Dart 150 Engine", "Tricon Zephyr Engine" }
end
eeng["Courier"] = neng["Courier"]
eeng["Freighter"] = neng["Freighter"]
eeng["Armoured Transport"] = neng["Armoured Transport"]

-- ELITE SETS
cores.elite = {
   systems = esys,
   hulls   = ehul,
   engines = eeng,
}

--[[
cores.get( "Fighter", { all="elite" } )
cores.get( "Fighter", { all={"normal","elite"}, heavy=true } )
cores.get( "Fighter", { systems="elite", hulls="normal", engines="elite" } )
--]]
function cores.get( shipclass, params )
   if params == nil then
      return nil
   end

   -- Check out if we have to do heavy
   local heavy
   if params.heavy == nil then
      heavy = (rnd.rnd() > 0.5)
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
      table.insert( c, cores[ systems ].systems[ shipclass ]( heavy ) )
   end
   if hulls then
      table.insert( c, cores[ hulls ].hulls[ shipclass ]( heavy ) )
   end
   if engines then
      table.insert( c, cores[ engines ].engines[ shipclass ]( heavy ) )
   end

   return c
end

return cores
