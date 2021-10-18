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

-- Text

function create ()
   -- Make sure system isn't claimed, but we don't claim it
   if not evt.claim( system.cur(), true ) then evt.finish() end

    -- The _("Shipwrecked %s") will be a random trader vessel.
    local r = rnd.rnd()
    if r > 0.95 then
        ship = "Gawain"
    elseif r > 0.8 then
        ship = "Mule"
    elseif r > 0.5 then
        ship = "Koala"
    else
        ship = "Llama"
    end

    -- Create the derelict.
    local pos   = vec2.newP( rnd.rnd(2000,3000), rnd.rnd()*360 )
    derelict    = pilot.add( ship, "Derelict", pos, nil, {ai="dummy"} )
    derelict:disable()
    derelict:rename(_("Shipwrecked %s"):format(_("August")))
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
    derelict:broadcast( string.format(_("SOS. This is %s. We are shipwrecked. Please #bboard#0 us by positioning your ship over ours and then #bdouble-clicking#0 on our ship."), _("August")), true )
    timer_delay = timer_delay or 10
    timer_delay = timer_delay + 5
    bctimer = hook.timer(timer_delay, "broadcast")
end

function rescue ()
    -- Player boards the _("Shipwrecked %s") and rescues the crew, this spawns a new mission.
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
