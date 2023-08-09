--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Proteron Blockade">
 <location>enter</location>
 <chance>100</chance>
 <cond>system.cur() == system.get("Leporis")</cond>
</event>
--]]

--[[
   Proteron Blockade

   This is just there to show off how the proteron are a bunch of fascist
   xenophobes who don't like outsiders. It also hides the fact that there is no
   content there (for now).
--]]

local pos_top = vec2.new(-15000, 2500)
local pos_bot = vec2.new(-11000, -6000)


local function spawn_fleet( pos )
   local ships  = {}
   if rnd.rnd() < 0.5 then
      ships[1] = "Proteron Watson"
   else
      ships[1] = "Proteron Archimedes"
   end
   ships[2] = "Proteron Pythagoras"

   for i=1,rnd.rnd(1,2) do
      table.insert( ships, "Proteron Gauss" )
   end
   for i=1,rnd.rnd(1,2) do
      table.insert( ships, "Proteron Hippocrates" )
   end

   local plts = {}
   local stand, _str = faction.get("Proteron"):playerStanding()
   for k,s in ipairs(ships) do
      local leader = plts[1]
      local p = pilot.add( s, "Proteron", pos, nil, {ai="guard"} )
      local aimem = p:memory()
      aimem.enemyclose    = 10e3
      aimem.guarddodist   = 10e3
      aimem.guardreturndist = 15e3
      p:setHostile(stand<0)
      if leader then
         p:setLeader( leader )
      end
      table.insert( plts, p )
      pos = pos + vec2.newP( 300, rnd.angle() )
   end
   return plts
end

function create ()
   local proteron_blockade = {}
   local n = rnd.rnd(5,6)
   for i=1,n do
      local pos = pos_top + (pos_bot - pos_top) * (i-1) / (n-1)
      local plts = spawn_fleet( pos )
      for k,v in ipairs(plts) do
         table.insert( proteron_blockade, v )
      end
   end

   hook.timer(3, "heartbeat", proteron_blockade )
   hook.jumpout("cleanup")
   hook.land("cleanup")
end

function heartbeat( proteron_blockade )
   local pp = player.pilot()
   local stand, _str = faction.get("Proteron"):playerStanding()
   for k,p in ipairs(proteron_blockade) and stand < 0 do
      if p:inrange( pp ) then
         p:broadcast( _("Unknown vessel trying to breach blockade. All ships engage!"), true)
         return
      end
   end

   hook.timer( 0.5, "heartbeat", proteron_blockade )
end

function cleanup ()
   evt.finish()
end
