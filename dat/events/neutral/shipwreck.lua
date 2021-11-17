--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Shipwreck">
  <trigger>enter</trigger>
  <chance>3</chance>
  <cond>require("common.pirate").systemPresence() &gt; 0</cond>
  <flags>
   <unique />
  </flags>
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
-- luacheck: globals broadcast destroyevent endevent rescue (Hook functions passed by name)

function create ()
   -- Make sure system isn't claimed, but we don't claim it
   if not evt.claim( system.cur(), true ) then evt.finish() end

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
    derelict    = pilot.add( dship, "Derelict", pos, nil, {ai="dummy"} )
    derelict:disable()
    derelict:rename(fmt.f(_("Shipwrecked {plt}"), {plt=shipname}))
    -- Added extra visibility for big systems (A.)
    derelict:setVisplayer( true )
    derelict:setHilight( true )

    hook.timer(3.0, "broadcast")

    -- Set hooks
    hook.pilot( p, "board", "rescue" )
    hook.pilot( p, "death", "destroyevent" )
    hook.enter("endevent")
    hook.land("endevent")
end

function broadcast ()
    -- Ship broadcasts an SOS every 10 seconds, until boarded or destroyed.
    if not derelict:exists() then
        return
    end
    derelict:broadcast( fmt.f(_("SOS. This is {plt}. We are shipwrecked. Please #bboard#0 us by positioning your ship over ours and then #bdouble-clicking#0 on our ship."), {plt=shipname}), true )
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

function destroyevent ()
    evt.finish(true)
 end

function endevent ()
    evt.finish()
end
