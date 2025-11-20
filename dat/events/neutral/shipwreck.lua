--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Shipwreck">
 <location>enter</location>
 <chance>3</chance>
 <cond>
   local cursys = system.cur()

   -- Avoid restricted systems
   if cursys:tags().restricted then
      return false
   end

   -- Don't do volatile systems
   local _nebu_dens, nebu_vol = cursys:nebula()
   if nebu_vol &gt; 0 then
      return false
   end

   -- Must have pirates, but not too much
   local pir = require("common.pirate").systemPresence()
   return pir &gt; 0 and pir &lt; 300
 </cond>
 <unique />
 <notes>
  <tier>1</tier>
 </notes>
</event>
--]]
--[[
-- Shipwreck Event
--
-- Creates a wrecked ship that asks for help. If the player boards it, the event switches to the Space Family mission.
-- See dat/missions/neutral/spacefamily
--
-- 12/02/2010 - Added visibility/highlight options for use in big systems (Anatolis)
--]]

local fmt = require "format"

local shipname = _("August")
local bctimer, derelict, timer_delay -- Non-persistent state

function create ()
   local cursys = system.cur()

   -- Make sure system isn't claimed, but we don't claim it
   if not naev.claimTest( cursys ) then evt.finish() end

   -- The _("Shipwrecked {plt}") will be a random trader vessel.
   local r = rnd.rnd()
   local dship
   if r > 0.95 then
      dship = "Gawain"
   elseif r > 0.8 then
      dship = "Mule"
   elseif r > 0.5 then
      dship = "Koala"
   else
      dship = "Llama"
   end

   -- Create the derelict.
   local pos   = vec2.newP( rnd.rnd(2000,3000), rnd.angle() )
   derelict    = pilot.add( dship, "Derelict", pos, fmt.f(_("Shipwrecked {plt}"), {plt=shipname}), {ai="dummy"} )
   derelict:setDisable()
   -- Added extra visibility for big systems (A.)
   derelict:setVisplayer( true )
   derelict:setHilight( true )

   hook.timer(3.0, "broadcast")

   -- Set hooks
   hook.pilot( derelict, "board", "rescue" )
   hook.pilot( derelict, "death", "destroyevent" )
   hook.enter("endevent")
   hook.land("endevent")
end

function broadcast ()
   -- Ship broadcasts an SOS every 10 seconds, until boarded or destroyed.
   if not derelict:exists() then
      return
   end
   derelict:broadcast( fmt.f(_("SOS. This is {plt}. We are shipwrecked. Please #bboard#0 us by #bdouble-clicking#0 on our ship."), {plt=shipname}), true )
   timer_delay = timer_delay or 10
   timer_delay = timer_delay + 5
   bctimer = hook.timer(timer_delay, "broadcast")
end

function rescue ()
   -- Player boards the _("Shipwrecked {plt}") and rescues the crew, this spawns a new mission.
   hook.rm(bctimer)
   naev.missionStart("The Space Family")
   evt.finish(true)
end

function destroyevent( _p, killer )
   if killer:faction() == player.pilot():faction() then
      evt.finish(true)
   end
   evt.finish()
end

function endevent ()
   evt.finish()
end
