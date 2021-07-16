--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Shipwreck">
  <trigger>enter</trigger>
  <chance>3</chance>
  <cond>system.cur():presence("Pirate") &gt; 0</cond>
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
broadcastmsg = _("SOS. This is %s. We are shipwrecked. Please #bboard#0 us by positioning your ship over ours and then #bdouble-clicking#0 on our ship.")
shipname = _("August") --The ship will have a unique name
shipwreck = _("Shipwrecked %s")

function create ()
    -- The shipwreck will be a random trader vessel.
    r = rnd.rnd()
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
    angle = rnd.rnd() * 2 * math.pi
    dist  = rnd.rnd(2000, 3000) -- place it a ways out
    pos   = vec2.new( dist * math.cos(angle), dist * math.sin(angle) )
    p     = pilot.add( ship, "Derelict", pos, nil, {ai="dummy"} )
    p:disable()
    p:rename(shipwreck:format(shipname))
    -- Added extra visibility for big systems (A.)
    p:setVisplayer( true )
    p:setHilight( true )

    hook.timer(3000, "broadcast")

    -- Set hooks
    hook.pilot( p, "board", "rescue" )
    hook.pilot( p, "death", "destroyevent" )
    hook.enter("endevent")
    hook.land("endevent")
end

function broadcast ()
    -- Ship broadcasts an SOS every 10 seconds, until boarded or destroyed.
    if not p:exists() then
        return
    end
    p:broadcast( string.format(broadcastmsg, shipname), true )
    bctimer = hook.timer(15000, "broadcast")
end

function rescue ()
    -- Player boards the shipwreck and rescues the crew, this spawns a new mission.
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
