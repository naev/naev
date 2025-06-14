--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Proteron Blockade">
 <location>enter</location>
 <chance>100</chance>
 <system>Leporis</system>
</event>
--]]

--[[
   Proteron Blockade

   This is just there to show off how the proteron are a bunch of fascist
   xenophobes who don't like outsiders. It also hides the fact that there is no
   content there (for now).
--]]

local jmp = jump.get( "Leporis", "Haered" )
local rad = 8000

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
   for k,s in ipairs(ships) do
      local leader = plts[1]
      local p = pilot.add( s, "Proteron", pos, nil, {ai="guard"} )
      local aimem = p:memory()
      aimem.enemyclose  = 10e3
      aimem.guarddodist = 10e3
      aimem.guardreturndist = 15e3
      aimem.capturable  = true
      if leader then
         p:setLeader( leader )
      end
      table.insert( plts, p )
      pos = pos + vec2.newP( 300, rnd.angle() )
   end
   return plts
end

function create ()
   local jmppos   = jmp:pos()
   local jmpang   = jmppos:angle()
   local fan      = math.rad(60)

   local proteron_blockade = {}
   local n = rnd.rnd(5,6)
   for i=1,n do
      local pos = jmppos - vec2.newP( rad, jmpang - fan + 2*fan*(i-1)/(n-1) )
      local plts = spawn_fleet( pos )
      for k,v in ipairs(plts) do
         table.insert( proteron_blockade, v )
      end
   end

   hook.timer(3, "heartbeat", proteron_blockade )
   hook.jumpout("cleanup")
   hook.land("cleanup")
end

local timer = 0
function heartbeat( proteron_blockade )
   -- If not hostile, ignore
   if not faction.get("Proteron"):areEnemies( faction.player() ) then
      return
   end

   -- Have the first ship that sees the player broadcast
   local pp = player.pilot()
   local spotted = false
   for k,p in ipairs(proteron_blockade) do
      if p:inrange( pp ) and p:flags("combat") then
         spotted = true
         if timer < 3 then
            timer = timer + 1
            break
         end
         p:broadcast( _("Unknown vessel trying to breach blockade. All ships engage!"), true)
         return
      end
   end

   if not spotted then
      timer = 0
   end

   hook.timer( 1, "heartbeat", proteron_blockade )
end

function cleanup ()
   evt.finish()
end
