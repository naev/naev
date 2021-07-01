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

-- ELITE ENGINES
local eeng = {}
eeng["Fighter"] = function( heavy )
   if heavy then
      return "Tricon Zephyr Engine"
   else
      return "Tricon Zephyr II Engine"
   end
end
eeng["Bomber"] = function( heavy )
   return "Tricon Zephyr Engine"
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

-- Elite sets
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
