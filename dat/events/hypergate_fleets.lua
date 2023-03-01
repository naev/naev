--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Hypergate Fleets">
 <location>enter</location>
 <chance>100</chance>
 <chapter>1</chapter>
</event>
--]]
--[[
   Hypergate guard fleets.
--]]
local fleet = require "fleet"


local hypergates_list = {
   "Hypergate Dvaer", -- Dvaered
   "Hypergate Feye", -- Soromid
   "Hypergate Gamma Polaris", -- Empire
   "Hypergate Kiwi", -- Sirius
   "Hypergate Ruadan", -- Za'lek
   --"Hypergate NGC-14549", -- Pirate
   --"Hypergate Polaris", -- Ruined
}
local systems_list = {}
for k,h in ipairs(hypergates_list) do
   hypergates_list[k], systems_list[k] = spob.getS(h)
end

local fleets_list = {
   ["Za'lek"]  = {
      "Za'lek Hephaestus"
   },
   ["Dvaered"] = {
      "Dvaered Goddard",
      "Dvaered Vigilance",
      "Dvaered Vigilance",
      "Dvaered Ancestor",
      "Dvaered Ancestor",
      "Dvaered Vendetta",
      "Dvaered Vendetta",
      "Dvaered Vendetta",
   },
   ["Soromid"] = {
      "Soromid Arx",
      "Soromid Nyx",
      "Soromid Nyx",
      "Soromid Marauder",
      "Soromid Marauder",
   },
   ["Sirius"]  = {
      "Sirius Divinity",
      "Sirius Dogma",
      "Sirius Dogma",
   },
   ["Empire"]  = {
      "Empire Peacemaker",
      "Empire Hawking",
      "Empire Hawking",
      "Empire Pacifier",
      "Empire Pacifier",
   },
}

function create ()
   local scur = system.cur()
   -- Make sure system isn't claimed, but we don't claim it
   if not naev.claimTest( scur ) then evt.finish() end

   -- Only care if we're in a system with hypergates
   local sysid
   for k,h in ipairs(systems_list) do
      if h==scur then
         sysid = k
         break
      end
   end
   if not sysid then evt.finish() end

   local hypergate = spob.get( hypergates_list[sysid] )

   -- We assume dominant faction is the one we want here
   local sysfct = hypergate:faction()
   local id = sysfct:nameRaw()
   local ships = fleets_list[ id ]
   local pos = hypergate:pos() + vec2.newP( 300+700*rnd.rnd(), rnd.angle() )

   -- Find out some waypoints
   local wp = {}
   for i=1,rnd.rnd(7,10) do
      table.insert( wp, hypergate:pos() + vec2.newP( 300+2700*rnd.rnd(), rnd.angle() ) )
   end

   -- Create pilots and they should patrol around
   local pilots = fleet.add( 1, ships, sysfct, pos, nil, nil )
   for k,p in ipairs(pilots) do
      local m = p:memory()
      m.waypoints = wp
      --m.doscans = true
      m.loiter = math.huge
   end

   -- Event finishes when leaving system
   hook.land( "endevent" )
   hook.jumpout( "endevent" )
end

function endevent ()
   evt.finish()
end
