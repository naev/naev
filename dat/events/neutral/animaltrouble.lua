--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--

   Animal Trouble event

   Temporarily makes the player's ship behave erratically.
   This event occurs after the player has done the "Animal transport" mission.

--]]

lang = naev.lang()
if lang == "es" then
    -- not translated atm
else -- default english 
    text = {}
    text[1] = [[Suddenly, your instruments go haywire, and your ship careens out of control. The controls aren't responding! Something is wrong with your systems!]]
    text[2] = [[You've found the cause of the problem. One of the little rodents you transported for that Sirian apparently got out of the crate on the way, and gnawed through some of your ship's circuitry. The creature died in the ensuing short-circuit. You've fixed the damage, and your ship is under control again.]]

    title = {}
    title[1] = "Panic!"
    title[2] = "Calm"
end 


function create ()
    -- Allow some time before the problems start
    hook.timer(45000, "startProblems")
    bucks = 6
end

function startProblems()
    -- Cancel autonav.
    player.cinematics(true)
    player.cinematics(false)
    tk.msg(title[1], text[1])
    ps = player.pilot()
    ps:control()
    hook.timer(7000, "buck")
    hook.pilot(ps, "idle", "continueProblems")
    continueProblems()
end

function continueProblems()
    -- Fly off in a random direction
    dist = 1000
    angle = rnd.rnd() * 90 + ps:dir() -- In theory, never deviate more than 90 degrees from the current course.
    newlocation = vec2.newP(dist, angle)

    ps:taskClear()
    ps:goto(ps:pos() + newlocation, false, false)
end

function buck()
    bucks = bucks -1
    if bucks == 0 then
        endProblems()
    end
    hook.timer(7000, "buck")
    continueProblems()
end

function endProblems()
    tk.msg(title[2], text[2])
    ps:control(false)
    var.pop("shipinfested")
    evt.finish(true)
end
