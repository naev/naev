--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Animal trouble">
  <trigger>enter</trigger>
  <chance>20</chance>
  <cond>var.peek("shipinfested") == true</cond>
  <flags>
   <unique />
  </flags>
  <notes>
   <done_misn name="Animal transport">The rodents sabotage your ship</done_misn>
  </notes>
 </event>
 --]]
--[[
-- Animal Trouble event
--
-- Temporarily makes the player's ship behave erratically.
-- This event occurs after the player has done the "Animal transport" mission.
--]]

require "missions/neutral/common"


text = {}
text[1] = _([[Suddenly, your instruments go haywire, and your ship careens out of control. The controls aren't responding! Something is wrong with your systems!]])
text[2] = _([[You've found the cause of the problem. One of the little rodents you transported for that Sirian apparently got out of the crate on the way, and gnawed through some of your ship's circuitry. The creature died in the ensuing short-circuit. You've fixed the damage, and your ship is under control again.]])

title = {}
title[1] = _("Panic!")
title[2] = _("Calm")

log_text = _([[You found that one of the rodents you transported for that Sirian got out of the crate on the way, gnawed through some of your ship's circuitry, and died from short-circuit caused by said gnawing, which also caused your ship to go haywire. After you fixed the damage, your ship's controls were brought back to normal.]])


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
    ps:moveto(ps:pos() + newlocation, false, false)
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
    addMiscLog( log_text )
    evt.finish(true)
end
