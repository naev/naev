--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--

   Shipwreck Event
 
   Creates a wrecked ship that asks for help. If the player boards it, the event switches to the Space Family mission.
   See dat/missions/neutral/spacefamily.lua
 
   12/02/2010 - Added visibility/highlight options for use in bigsystems (Anatolis)

--]]

lang = naev.lang()
if lang == "es" then
    -- not translated atm
else -- default english 

-- Text
    broadcastmsg = "SOS. This is %s. We are shipwrecked. Requesting immediate assistance."
    shipname = "August" --The ship will have a unique name
end

function create ()

    -- The shipwrech will be a random trader vessel.
    r = rnd.rnd()
    if r > 0.95 then
        ship = "Trader Gawain"
    elseif r > 0.8 then
        ship = "Trader Mule"
    elseif r > 0.5 then
        ship = "Trader Koala"
    else 
        ship = "Trader Llama"
    end

    -- Create the derelict.
    angle = rnd.rnd() * 2 * math.pi
    dist  = rnd.rnd(2000, 3000) -- place it a ways out
    pos   = vec2.new( dist * math.cos(angle), dist * math.sin(angle) )
    p     = pilot.add(ship, "dummy", pos)
    for k,v in ipairs(p) do
        v:setFaction("Derelict")
        v:disable()
        v:rename("Shipwrecked " .. shipname)
        -- Added extra visibility for big systems (A.)
        v:setVisplayer( true )
        v:setHilight( true )
    end

    hook.timer(3000, "broadcast")
   
    -- Set hooks
    hook.pilot( p[1], "board", "rescue" )
    hook.pilot( p[1], "death", "destroyevent" )
    hook.enter("endevent")
    hook.land("endevent")
end

function broadcast()
    -- Ship broadcasts an SOS every 10 seconds, until boarded or destroyed.
    if not p[1]:exists() then
       return
    end
    p[1]:broadcast( string.format(broadcastmsg, shipname), true )
    bctimer = hook.timer(15000, "broadcast")
end

function rescue()
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
