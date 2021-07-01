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
