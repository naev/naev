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


local having_secondary={"Fighter","Bomber","Destroyer","Battleship","Carrier","Courier","Freighter","Armoured Transport","Bulk Freighter"}
local have_secondary={}

for _i,x in ipairs(having_secondary) do
   have_secondary[x]=true
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
   return "Unicorp PT-16 Core System"
end
ssys["Bomber"] = function ()
   return "Unicorp PT-16 Core System"
end
ssys["Corvette"] = function ()
   return "Unicorp PT-200 Core System"
end
ssys["Destroyer"] = function ()
   return "Unicorp PT-200 Core System"
end
ssys["Cruiser"] = function ()
   return "Unicorp PT-440 Core System"
end
ssys["Battleship"] = function ()
   return "Unicorp PT-440 Core System"
end
ssys["Carrier"] = function ()
   return "Unicorp PT-440 Core System"
end
-- Civilian
ssys["Yacht"] = function ()
   return "Unicorp PT-16 Core System"
end
ssys["Courier"] = function ()
   return "Unicorp PT-16 Core System"
end
ssys["Freighter"] = function ()
   return "Unicorp PT-200 Core System"
end
ssys["Armoured Transport"] = ssys["Freighter"]
ssys["Bulk Freighter"] = function ()
   return "Unicorp PT-440 Core System"
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
   return "Unicorp D-2 Light Plating"
end
shul["Bomber"] = function ()
   return "Unicorp D-2 Light Plating"
end
shul["Corvette"] = function ()
   return "Unicorp D-23 Medium Plating"
end
shul["Destroyer"] = function ()
   return "Unicorp D-23 Medium Plating"
end
shul["Cruiser"] = function ()
   return "Unicorp D-58 Heavy Plating"
end
shul["Battleship"] = function ()
   return "Unicorp D-58 Heavy Plating"
end
shul["Carrier"] = function ()
   return "Unicorp D-58 Heavy Plating"
end
-- Civilian
shul["Yacht"] = function ()
   return choose_one{ "Unicorp D-2 Light Plating", "S&K Small Cargo Hull" }
end
shul["Courier"] = function ()
   return choose_one{ "Unicorp D-2 Light Plating", "S&K Small Cargo Hull" }
end
shul["Freighter"] = function ()
   return choose_one{ "Unicorp D-23 Medium Plating", "S&K Medium Cargo Hull" }
end
shul["Armoured Transport"] = shul["Freighter"]
shul["Bulk Freighter"] = function ()
   return "Unicorp D-58 Heavy Plating"
end

-- STANDARD ENGINES
local seng = {}
seng["Scout"] = function ()
   return "Unicorp Hawk 160 Engine"
end
seng["Interceptor"] = function ()
   return "Unicorp Hawk 160 Engine"
end
seng["Fighter"] = function ()
   return "Unicorp Hawk 160 Engine"
end
seng["Bomber"] = function ()
   return "Unicorp Hawk 160 Engine"
end
seng["Corvette"] = function ()
   return "Unicorp Falcon 700 Engine"
end
seng["Destroyer"] = function ()
   return "Unicorp Falcon 700 Engine"
end
seng["Cruiser"] = function ()
   return "Unicorp Eagle 3000 Engine"
end
seng["Battleship"] = function ()
   return "Unicorp Eagle 3000 Engine"
end
seng["Carrier"] = function ()
   return "Unicorp Eagle 3000 Engine"
end
seng["Yacht"] = function ()
   return "Unicorp Hawk 160 Engine"
end
seng["Courier"] = function ()
   return "Unicorp Hawk 160 Engine"
end
seng["Freighter"] = function ()
   return choose_one{ "Unicorp Falcon 700 Engine", "Melendez Buffalo Engine" }
end
seng["Armoured Transport"] = seng["Freighter"]
seng["Bulk Freighter"] = function ()
   return choose_one{ "Unicorp Eagle 3000 Engine", "Melendez Mammoth Engine" }
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
   return choose_one{ "Milspec Orion 2301 Core System", "Milspec Aegis 2201 Core System", "Milspec Thalos 2202 Core System", "Milspec Prometheus 2203 Core System" }
end
esys["Fighter"] = function ()
   return choose_one{ "Milspec Orion 2301 Core System", "Milspec Aegis 2201 Core System", "Milspec Thalos 2202 Core System", "Milspec Prometheus 2203 Core System" }
end
esys["Bomber"] = function ()
   return choose_one{ "Milspec Orion 2301 Core System", "Milspec Aegis 2201 Core System" }
end
esys["Corvette"] = function ()
   return choose_one{ "Milspec Orion 4801 Core System", "Milspec Aegis 4701 Core System", "Milspec Thalos 4702 Core System", "Milspec Prometheus 4703 Core System" }
end
esys["Destroyer"] = function ()
   return choose_one{ "Milspec Orion 4801 Core System", "Milspec Aegis 4701 Core System", "Milspec Thalos 4702 Core System", "Milspec Prometheus 4703 Core System" }
end
esys["Cruiser"] = function ()
   return choose_one{ "Milspec Orion 8601 Core System", "Milspec Aegis 8501 Core System", "Milspec Thalos 8502 Core System", "Milspec Prometheus 8503 Core System" }
end
esys["Battleship"] = function ()
   return choose_one{ "Milspec Orion 8601 Core System", "Milspec Aegis 8501 Core System", "Milspec Thalos 8502 Core System", "Milspec Prometheus 8503 Core System" }
end
esys["Carrier"] = function ()
   return choose_one{ "Milspec Orion 8601 Core System", "Milspec Aegis 8501 Core System", "Milspec Thalos 8502 Core System", "Milspec Prometheus 8503 Core System" }
end
-- Civilian
esys["Yacht"] = ssys["Yacht"]
esys["Courier"] = ssys["Courier"]
esys["Freighter"] = ssys["Freighter"]
esys["Armoured Transport"] = function ()
   return choose_one{ "Milspec Orion 4801 Core System", "Milspec Aegis 4701 Core System", "Milspec Prometheus 4703 Core System" }
end
esys["Bulk Freighter"] = function ()
   return choose_one{ "Milspec Orion 8601 Core System", "Milspec Aegis 8501 Core System", "Milspec Prometheus 8503 Core System" }
end

-- ELITE HULLS
local ehul = {}
ehul["Scout"] = function ()
   return choose_one{ "Nexus Shadow Weave", "S&K Skirmish Plating" }
end
ehul["Interceptor"] = function ()
   return choose_one{ "Nexus Shadow Weave", "S&K Skirmish Plating" }
end
ehul["Fighter"] = function (flag)
   if flag then
      return "S&K Skirmish Plating"
   else
      return choose_one{ "Nexus Shadow Weave", "S&K Skirmish Plating" }
   end
end
ehul["Bomber"] = ehul["Fighter"]
ehul["Corvette"] = function ()
   return choose_one{ "Nexus Ghost Weave", "S&K Battle Plating" }
end
ehul["Destroyer"] = function ()
   return "S&K Battle Plating"
end
ehul["Cruiser"] = function ()
   return "S&K War Plating"
end
ehul["Battleship"] = function ()
   return "S&K War Plating"
end
ehul["Carrier"] = function ()
   return "S&K War Plating"
end
-- Civilian
ehul["Yacht"] = shul["Yacht"]
ehul["Courier"] = shul["Courier"]
ehul["Freighter"] = shul["Freighter"]
ehul["Armoured Transport"] = function (flag)
   if flag then
      return "Unicorp D-23 Medium Plating"
   else
      return choose_one{ "Unicorp D-23 Medium Plating", "Nexus Ghost Weave" }
   end
end
ehul["Bulk Freighter"] = function (flag)
   if flag then
      return "Unicorp D-58 Heavy Plating"
   else
      return "S&K Large Cargo Hull"
   end
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
   return choose_one{ "Nexus Dart 160 Engine", "Tricon Zephyr Engine" }
end
eeng["Bomber"] = function ()
   return choose_one{ "Nexus Dart 160 Engine", "Melendez Ox Engine" }
end
eeng["Corvette"] = function ()
   return choose_one{ "Tricon Cyclone Engine", "Nexus Arrow 700 Engine", "Melendez Buffalo Engine" }
end
eeng["Destroyer"] = function ()
   return choose_one{ "Tricon Cyclone Engine", "Nexus Arrow 700 Engine", "Melendez Buffalo Engine" }
end
eeng["Cruiser"] = function ()
   return choose_one{ "Tricon Typhoon Engine", "Nexus Bolt 3000 Engine", "Melendez Mammoth Engine" }
end
eeng["Battleship"] = function ()
   return choose_one{ "Tricon Typhoon Engine", "Nexus Bolt 3000 Engine", "Melendez Mammoth Engine" }
end
eeng["Carrier"] = function ()
   return "Melendez Mammoth Engine"
end
eeng["Yacht"] = seng["Yacht"]
eeng["Courier"] = seng["Courier"]
eeng["Freighter"] = seng["Freighter"]
eeng["Armoured Transport"] = seng["Armoured Transport"]
eeng["Bulk Freighter"] = function ()
   return "Melendez Mammoth Engine"
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
   return "Krain Remige Engine"
end
cores.standard.engines["Pirate Kestrel"] = cores.standard.engines["Kestrel"]
cores.standard.engines["Starbridge"] = function ()
   return choose_one{ "Unicorp Falcon 700 Engine", "Krain Patagium Twin Engine" }
end
cores.standard.engines["Pirate Starbridge"] = cores.standard.engines["Starbridge"]
cores.standard.systems["Proteron Hippocrates"] = function ()
   return "Milspec Thalos 4702 Core System"
end

-- Elite exceptions
cores.elite.engines["Kestrel"] = function ()
   return "Krain Remige Engine"
end
cores.elite.engines["Pirate Kestrel"] = cores.elite.engines["Kestrel"]
cores.elite.hulls["Kestrel"] = function ()
   return "Unicorp D-58 Heavy Plating"
end
cores.elite.hulls["Pirate Kestrel"] = cores.elite.hulls["Kestrel"]
cores.elite.engines["Starbridge"] = function ()
   return choose_one{ "Tricon Cyclone Engine", "Krain Patagium Twin Engine" }
end
cores.elite.engines["Pirate Starbridge"] = cores.elite.engines["Starbridge"]
cores.elite.systems["Gawain"] = function ()
   return choose_one{ "Unicorp PT-16 Core System", "Milspec Orion 2301 Core System" }
end
cores.elite.hulls["Gawain"] = function ()
   return choose_one{ "Unicorp D-2 Light Plating", "Nexus Shadow Weave" }
end
cores.elite.engines["Gawain"] = function ()
   return choose_one{ "Nexus Dart 160 Engine", "Tricon Zephyr Engine" }
end
cores.elite.systems["Proteron Hippocrates"] = function ()
   return "Milspec Thalos 4702 Core System"
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
      c["systems"] = co()
      if have_secondary[shipclass] then
         c["systems_secondary"] = co()
      end
   end
   if hulls then
      local ct = cores[ hulls ].hulls
      local co = ct[ shipname ] or ct[ shipclass ]
      c["hull"] = co()
      if have_secondary[shipclass] then
         c["hull_secondary"] = co(true)
      end
   end
   if engines then
      local ct = cores[ engines ].engines
      local co = ct[ shipname ] or ct[ shipclass ]
      c["engines"] = co()
      if have_secondary[shipclass] then
         c["engines_secondary"] = c["engines"]
      end
   end
   --acc = tostring(s) .. ': {'
   --for k,v in pairs(c) do
   --   acc = acc .. ' ' .. tostring(k) .. ':' .. tostring(v)
   --end
   --print(acc .. '}')

   return c
end

return cores
