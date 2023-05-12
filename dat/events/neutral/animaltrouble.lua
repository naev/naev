--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Animal trouble">
 <location>enter</location>
 <chance>20</chance>
 <cond>var.peek("shipinfested") == true</cond>
 <unique />
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
local neu = require "common.neutral"
local vntk = require "vntk"

function create ()
    -- Allow some time before the problems start
    hook.timer(45.0, "startProblems")
end

function startProblems()
    -- Cancel autonav.
    player.autonavAbort()
    vntk.msg(_("Panic!"), _([[Suddenly, your instruments go haywire, and your ship careens out of control. The controls aren't responding! Something is wrong with your systems!]]))
    local ps = player.pilot()
    ps:control()
    hook.timer(7.0, "buck", 5)
    hook.pilot(ps, "idle", "continueProblems")
    continueProblems()
end

function continueProblems()
    -- Fly off in a random direction
    local ps = player.pilot()
    local dist = 1000
    local angle = rnd.rnd() * math.pi/2 + ps:dir() -- In theory, never deviate more than 90 degrees from the current course.
    local newlocation = vec2.newP(dist, angle)

    ps:taskClear()
    ps:moveto(ps:pos() + newlocation, false, false)
end

local function endProblems()
    vntk.msg(_("Calm"), _([[You've found the cause of the problem. One of the little rodents you transported for that Sirian apparently got out of the crate on the way, and gnawed through some of your ship's circuitry. The creature died in the ensuing short-circuit. You've fixed the damage, and your ship is under control again.]]))
    player.pilot():control(false)
    var.pop("shipinfested")
    neu.addMiscLog( _([[You found that one of the rodents you transported for that Sirian got out of the crate on the way, gnawed through some of your ship's circuitry, and died from short-circuit caused by said gnawing, which also caused your ship to go haywire. After you fixed the damage, your ship's controls were brought back to normal.]]) )
    evt.finish(true)
end

function buck(bucks)
    if bucks == 0 then
        endProblems()
    end
    hook.timer(7.0, "buck", bucks-1)
    continueProblems()
end
