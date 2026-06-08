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
local function systems_unicorp( _secondary, size )
   if size=="Large" then
      return "Unicorp PT-440 Core System"
   elseif size=="Medium" then
      return "Unicorp PT-200 Core System"
   else
      return "Unicorp PT-16 Core System"
   end
end
local ssys = {}
ssys["Scout"]     = systems_unicorp
ssys["Interceptor"] = systems_unicorp
ssys["Fighter"]   = systems_unicorp
ssys["Bomber"]    = systems_unicorp
ssys["Corvette"]  = systems_unicorp
ssys["Destroyer"] = systems_unicorp
ssys["Cruiser"]   = systems_unicorp
ssys["Battleship"] = systems_unicorp
ssys["Carrier"]   = systems_unicorp
-- Civilian
ssys["Yacht"]     = systems_unicorp
ssys["Courier"]   = systems_unicorp
ssys["Freighter"] = systems_unicorp
ssys["Armoured Transport"] = systems_unicorp
ssys["Bulk Freighter"] = systems_unicorp

-- STANDARD HULLS
local function hulls_unicorp( _secondary, size )
   if size=="Large" then
      return "Unicorp D-58 Heavy Plating"
   elseif size=="Medium" then
      return "Unicorp D-23 Medium Plating"
   else
      return "Unicorp D-2 Light Plating"
   end
end
local function hulls_unicorp_cargo( _secondary, size )
   if size=="Large" then
      return choose_one{ "Unicorp D-58 Heavy Plating", "S&K Large Cargo Hull" }
   elseif size=="Medium" then
      return choose_one{ "Unicorp D-23 Medium Plating", "S&K Medium Cargo Hull" }
   else
      return choose_one{ "Unicorp D-2 Light Plating", "S&K Small Cargo Hull" }
   end
end
local shul = {}
shul["Scout"]        = hulls_unicorp
shul["Interceptor"]  = hulls_unicorp
shul["Fighter"]      = hulls_unicorp
shul["Bomber"]       = hulls_unicorp
shul["Corvette"]     = hulls_unicorp
shul["Destroyer"]    = hulls_unicorp
shul["Cruiser"]      = hulls_unicorp
shul["Battleship"]   = hulls_unicorp
shul["Carrier"]      = hulls_unicorp
-- Civilian
shul["Yacht"]        = hulls_unicorp_cargo
shul["Courier"]      = hulls_unicorp_cargo
shul["Freighter"]    = hulls_unicorp_cargo
shul["Armoured Transport"] = hulls_unicorp_cargo
shul["Bulk Freighter"] = hulls_unicorp_cargo

-- STANDARD ENGINES
local function engines_unicorp( _secondary, size )
   if size=="Large" then
      return "Unicorp Eagle 3000 Engine"
   elseif size=="Medium" then
      return "Unicorp Falcon 700 Engine"
   else
      return "Unicorp Hawk 160 Engine"
   end
end
local function engines_unicorp_melendez( _secondary, size )
   if size=="Large" then
      return choose_one{ "Unicorp Eagle 3000 Engine", "Melendez Mammoth Engine" }
   elseif size=="Medium" then
      return choose_one{ "Unicorp Falcon 700 Engine", "Melendez Buffalo Engine" }
   else
      return choose_one{ "Unicorp Hawk 160 Engine", "Melendez Ox Engine" }
   end
end
local seng = {}
seng["Scout"]        = engines_unicorp
seng["Interceptor"]  = engines_unicorp
seng["Fighter"]      = engines_unicorp
seng["Bomber"]       = engines_unicorp
seng["Corvette"]     = engines_unicorp
seng["Destroyer"]    = engines_unicorp
seng["Cruiser"]      = engines_unicorp
seng["Battleship"]   = engines_unicorp
seng["Carrier"]      = engines_unicorp
seng["Yacht"]        = engines_unicorp
seng["Courier"]      = engines_unicorp_melendez
seng["Freighter"]    = engines_unicorp_melendez
seng["Armoured Transport"] = engines_unicorp_melendez
seng["Bulk Freighter"] = engines_unicorp_melendez

-- STANDARD SETS
cores.standard = {
   systems = ssys,
   hulls   = shul,
   engines = seng,
}

--[[
      ELITE CORES
--]]
local function systems_all_elite( _secondary, size )
   if size=="Large" then
      return choose_one{ "Milspec Orion 8601 Core System", "Milspec Aegis 8501 Core System", "Milspec Thalos 8502 Core System", "Milspec Prometheus 8503 Core System" }
   elseif size=="Medium" then
      return choose_one{ "Milspec Orion 4801 Core System", "Milspec Aegis 4701 Core System", "Milspec Thalos 4702 Core System", "Milspec Prometheus 4703 Core System" }
   else
      return choose_one{ "Milspec Orion 2301 Core System", "Milspec Aegis 2201 Core System", "Milspec Thalos 2202 Core System", "Milspec Prometheus 2203 Core System" }
   end
end
local function systems_all_but_thalos_elite( _secondary, size )
   if size=="Large" then
      return choose_one{ "Milspec Orion 8601 Core System", "Milspec Aegis 8501 Core System", "Milspec Prometheus 8503 Core System" }
   elseif size=="Medium" then
      return choose_one{ "Milspec Orion 4801 Core System", "Milspec Aegis 4701 Core System", "Milspec Prometheus 4703 Core System" }
   else
      return choose_one{ "Milspec Orion 2301 Core System", "Milspec Aegis 2201 Core System", "Milspec Prometheus 2203 Core System" }
   end
end
-- ELITE SYSTEMS
local esys = {}
esys["Scout"] = function ()
   return "Milspec Orion 2301 Core System"
end
esys["Interceptor"]  = systems_all_elite
esys["Fighter"]      = systems_all_elite
esys["Bomber"] = function ()
   return choose_one{ "Milspec Orion 2301 Core System", "Milspec Aegis 2201 Core System" }
end
esys["Corvette"]     = systems_all_elite
esys["Destroyer"]    = systems_all_elite
esys["Cruiser"]      = systems_all_elite
esys["Battleship"]   = systems_all_elite
esys["Carrier"]      = systems_all_elite -- TODO prefer Thalos?
-- Civilian
esys["Yacht"]        = systems_all_but_thalos_elite
esys["Courier"]      = systems_all_but_thalos_elite
esys["Freighter"]    = systems_all_but_thalos_elite
esys["Armoured Transport"] = systems_all_but_thalos_elite
esys["Bulk Freighter"] = systems_all_but_thalos_elite

-- ELITE HULLS
local function hulls_sk_nexus( _secondary, size )
   if size=="Large" then
      return choose_one{ "Nexus Phantasm Weave", "S&K War Plating" }
   elseif size=="Medium" then
      return choose_one{ "Nexus Ghost Weave", "S&K Battle Plating" }
   else
      return choose_one{ "Nexus Shadow Weave", "S&K Skirmish Plating" }
   end
end
local function hulls_sk( _secondary, size )
   if size=="Large" then
      return "S&K War Plating"
   elseif size=="Medium" then
      return "S&K Battle Plating"
   else
      return "S&K Skirmish Plating"
   end
end
local function hulls_nexus_rs_cargo( _secondary, size )
   if size=="Large" then
      return choose_one{ "Nexus Phantasm Weave", "Red Star Large Cargo Hull", "S&K Large Cargo Hull" }
   elseif size=="Medium" then
      return choose_one{ "Nexus Ghost Weave", "Red Star Medium Cargo Hull", "S&K Medium Cargo Hull" }
   else
      return choose_one{ "Nexus Shadow Weave", "Red Star Small Cargo Hull", "S&K Small Cargo Hull" }
   end
end
local ehul = {}
ehul["Scout"]        = hulls_sk_nexus
ehul["Interceptor"]  = hulls_sk_nexus
ehul["Fighter"]      = function ( secondary, size )
   if secondary then
      return hulls_sk( secondary, size )
   else
      return hulls_sk_nexus( secondary, size )
   end
end
ehul["Bomber"]       = ehul["Fighter"]
ehul["Corvette"]     = hulls_sk_nexus
ehul["Destroyer"]    = hulls_sk
ehul["Cruiser"]      = hulls_sk
ehul["Battleship"]   = hulls_sk
ehul["Carrier"]      = hulls_sk
-- Civilian
ehul["Yacht"]        = hulls_nexus_rs_cargo
ehul["Courier"]      = hulls_nexus_rs_cargo
ehul["Freighter"]    = hulls_nexus_rs_cargo
ehul["Armoured Transport"] = function ( secondary, size )
   if secondary then
      return hulls_sk( secondary, size )
   else
      return hulls_nexus_rs_cargo( secondary, size )
   end
end
ehul["Bulk Freighter"] = hulls_nexus_rs_cargo

-- ELITE ENGINES
local function engines_melendez( _secondary, size )
   if size=="Large" then
      return "Melendez Mammoth Engine"
   elseif size=="Medium" then
      return "Melendez Buffalo Engine"
   else
      return "Melendez Ox Engine"
   end
end
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
eeng["Carrier"]   = engines_melendez
eeng["Yacht"]     = engines_melendez
eeng["Courier"]   = engines_melendez
eeng["Freighter"] = engines_melendez
eeng["Armoured Transport"] = engines_melendez
eeng["Bulk Freighter"] = engines_melendez

-- ELITE SETS
cores.elite = {
   systems = esys,
   hulls   = ehul,
   engines = eeng,
}

--[[
if __debugging then
   for k,v in ipairs{
      "Yacht",
      "Courier",
      "Freighter",
      "Armoured Transport",
      "Bulk Freighter",
      "Scout",
      "Interceptor",
      "Fighter",
      "Bomber",
      "Corvette",
      "Destroyer",
      "Cruiser",
      "Battleship",
      "Carrier",
   } do
      for i,s in ipairs{
         "systems",
         "hulls",
         "engines",
      } do
         assert( cores.standard[s][v] ~= nil, v.."-"..s.."-standard" )
         assert( cores.elite[s][v] ~= nil, v.."-"..s.."-elite" )
      end
   end
end
--]]

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
cores.elite.hulls["Starbridge"] = function (secondary)
   if secondary then
      return choose_one{ "S&K Battle Plating", "Krain Nanotubes Foam Padding" }
   else
      return "S&K Battle Plating"
   end
end
cores.elite.hulls["Pirate Starbridge"] = cores.elite.hulls["Starbridge"]
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

   -- Lazy loading of slot info
   local nc = naev.cache()
   if nc.equipopt_slots == nil then
      nc.equipopt_slots = {}
      for k,ks in ipairs( ship.getAll() ) do
         local slots = ks:getSlots()
         local slotinfo = {}
         for i,v in ipairs(slots) do
            if v.property == "systems" then
               slotinfo.systems = slots.size
            elseif v.property == "engines" then
               slotinfo.engines = slots.size
            elseif v.property == "hull" then
               slotinfo.hull = slots.size
            elseif v.property == "systems_secondary" then
               slotinfo.systems_secondary = slots.size
            elseif v.property == "engines_secondary" then
               slotinfo.engines_secondary = slots.size
            elseif v.property == "hull_secondary" then
               slotinfo.hull_secondary = slots.size
            end
         end
         nc.equipopt_slots[ ks:nameRaw() ] = slotinfo
      end
   end
   local slotinfo = nc.equipopt_slots[ shipname ]

   -- Get the cores if applicable
   local c = {}
   if systems then
      local ct = cores[ systems ].systems
      local co = ct[ shipname ] or ct[ shipclass ]
      c["systems"] = co(false,slotinfo.systems)
      if slotinfo.systems_secondary then
         c["systems_secondary"] = co(true,slotinfo.systems_secondary)
      end
   end
   if hulls then
      local ct = cores[ hulls ].hulls
      local co = ct[ shipname ] or ct[ shipclass ]
      c["hull"] = co(false,slotinfo.hull)
      if slotinfo.hull_secondary then
         c["hull_secondary"] = co(true,slotinfo.hull_secondary)
      end
   end
   if engines then
      local ct = cores[ engines ].engines
      local co = ct[ shipname ] or ct[ shipclass ]
      -- Implicit assumption engines always match size
      c["engines"] = co(false,slotinfo.engines)
      if slotinfo.engines_secondary then
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
