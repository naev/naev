--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Onion Society 02 Trigger">
 <unique />
 <priority>3</priority>
 <location>enter</location>
 <chance>6</chance>
 <cond>
   local m = "Onion Society 02"
   if player.misnDone(m) or
      player.misnActive(m) then
      return false
   end
   local _spb, sys = spob.getS("Ulios")
   if sys:jumpDist() &gt; 15 then
      return false
   end
   local sf = system.cur():faction()
   if not inlist( {
      faction.get("Empire"),
      faction.get("Dvaered"),
      faction.get("Za'lek"),
   }, sf ) then
      return false
   end
   return true
 </cond>
 <notes>
  <campaign>Onion Society</campaign>
  <done_misn name="Onion Society 01" />
 </notes>
</event>
--]]
--[[
   Comm Event for the Baron mission string
--]]
local vn = require "vn"
--local fmt = require "format"
local onion = require "common.onion"

function create ()
   -- Inclusive claims, so not an issue they overlap with the mission itself
   if not evt.claim( system.cur(), true ) then
      return evt.finish()
   end

   hook.timer( 10 + 20*rnd.rnd(), "trigger" )
   hook.land("finish")
   hook.jumpout("finish")
end

-- Possesses a ship to get close to the player
local possessed
function trigger ()
   local plt = pilot.get( { faction.get("Independent") } )
   if #plt <= 0 then
      -- No valid pilots, so we failed
      return evt.finish()
   end
   local pos = player.pos()
   table.sort( plt, function (a, b)
      return pos:dist2(a:pos())-pos:dist2(b:pos())
   end )
   possessed = plt[1]

   possessed:control()
   possessed:follow( player.pilot() )
   possessed:effectAdd("Onionized")
   hook.pilot( possessed, "death", "trigger" ) -- Retrigger if dies
   hook.pilot( possessed, "hail", "hail" )
   hook.timer( 1, "heartbeat" )
end

-- Triggers when gets close to player
function heartbeat ()
   if not possessed or not possessed:exists() then
      return
   end

   -- Be nice to the player and hail them when close enough
   if possessed:pos():dist( player.pos() ) < 3e3 then
      possessed:hailPlayer()
      return -- No need for more heartbeat
   end

   hook.timer( 1, "heartbeat" )
end

-- Hailed by player
function hail()
   local accepted = false

   vn.clear()
   vn.scene()
   local o = vn.newCharacter( onion.vn_onion() )
   vn.transition("electric")
   vn.na(_([[You communicate with the strange ship.]]))
   o(_([[]]))
   vn.done("electric")
   vn.run()

   -- Reset the NPC
   possessed:effectRm("Onionized")
   possessed:control(false)

   -- Clean up and start mission
   player.commClose()
   if accepted then
      naev.missionStart("Onion Society 02")
      return evt.finish(false) -- Can't set to true in case the mission gets failed or whatever, so it can be started again
   else
      return evt.finish(false)
   end
end

function finish ()
   return evt.finish(false)
end
