--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Emergency of Immediate Inspiration">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <done>Advanced Nebula Research</done>
  <chance>30</chance>
  <location>Bar</location>
  <faction>Empire</faction>
  <cond>system.get("Gamma Polaris"):jumpDist() &lt; 3 and planet.cur():class() ~= "1" and planet.cur():class() ~= "2" and planet.cur():class() ~= "3"</cond>
 </avail>
 <notes>
  <campaign>Nebula Research</campaign>
 </notes>
</mission>
--]]
--[[

   Mission: Emergency of Immediate Inspiration

   Description: Take Dr. Mensing to Jorla as fast as possible!

   Difficulty: Easy

--]]
local car = require "common.cargo"
local fmt = require "format"
local zlk = require "common.zalek"

-- luacheck: globals land (Hook functions passed by name)

-- Mission constants
local credits = 400e3
local homeworld, homeworld_sys = planet.getS("Jorla")
local request_text = _([["There's actually another thing I've almost forgotten. I also have to attend another conference very soon on behalf of professor Voges who obviously is very busy with some project he would not tell me about. But I don't want to go there - my research is far too important! So could you instead bring Robert there? You remember the student you helped out recently? I'm sure he will do the presentation just fine! I'll tell him to meet you in the bar as soon as possible!"
    With that being said Dr. Mensing leaves you immediately without waiting for your answer. It appears you should head to the bar to meet up with the student.]])

function create()
    -- mission variables
    mem.origin = planet.cur()
    mem.origin_sys = system.cur()

    local numjumps = mem.origin_sys:jumpDist(homeworld_sys, false)
    local traveldist = car.calculateDistance(mem.origin_sys, mem.origin:pos(), homeworld_sys, homeworld)
    local stuperpx   = 0.15
    local stuperjump = 10000
    local stupertakeoff = 10000
    local allowance  = traveldist * stuperpx + numjumps * stuperjump + stupertakeoff + 240 * numjumps
    mem.timelimit  = time.get() + time.create(0, 0, allowance)

    -- Spaceport bar stuff
    misn.setNPC(_("Dr. Mensing"), "zalek/unique/mensing.webp", _("It appears she wants to talk with you."))
end

function accept()
    if not tk.yesno(_("Bar"), fmt.f(_([["Well met, {player}! In fact, it's a lucky coincidence that we meet. You see, I'm in dire need of your service. I'm here on a... conference of sorts, not a real one. We are obligated to present the newest results of our research to scientists of the Empire once per period - even though these jokers lack the skills to understand our works! It's just a pointless ritual anyway. But I just got an ingenious idea on how to prevent the volatile Sol nebula from disrupting ship shields! I will spare you with the details - to ensure my idea is not going to be stolen, nothing personal. You can never be sure who is listening."
    "Anyway, you have to take me back to my lab on {pnt} in the {sys} system immediately! I'd also pay {credits} if necessary."]]), {player=player.name(), pnt=homeworld, sys=homeworld_sys, credits=fmt.credits(credits)})) then
        misn.finish()
    end
    tk.msg(_("Bar"), fmt.f(_([["Splendid! I'd like to start with my work as soon as possible, so please hurry! Off to {pnt} we go!"
    With that being said she drags you out of the bar. When realizing that she actually does not know on which landing pad your ship is parked she lets you loose and orders you to lead the way.]]), {pnt=homeworld}))

    -- Set up mission information
    misn.setTitle(_("Emergency of Immediate Inspiration"))
    misn.setReward(fmt.credits(credits))
    misn.setDesc(fmt.f(_("Take Dr. Mensing to {pnt} in the {sys} system as fast as possible!"), {pnt=homeworld, sys=homeworld_sys}))
    mem.misn_marker = misn.markerAdd(homeworld, "low")

    misn.accept()
    misn.osdCreate(_("Emergency of Immediate Inspiration"), {
       fmt.f(_("Fly to {pnt} in the {sys} system."), {pnt=homeworld, sys=homeworld_sys}),
    })

    hook.land("land")
end

function land()
    mem.landed = planet.cur()
    if mem.landed == homeworld then
        if mem.timelimit < time.get() then
            tk.msg(_("Mission accomplished"), fmt.f(_([["That took long enough! I can't await getting started. I doubt you deserve full payment. I'll rather give you a reduced payment of {credits} for educational reasons." She hands you over a credit chip.]]), {credits=fmt.credits(credits / 2)} .. "\n\n" .. request_text))
            player.pay(credits / 2)
        else
            tk.msg(_("Mission accomplished"), fmt.f(_([["Finally! I can't await getting started. Before I forget -" She hands you over a credit chip worth {credits}.]]), {credits=fmt.credits(credits)}) .. "\n\n" .. request_text)
            player.pay(credits)
        end
        zlk.addNebuResearchLog(_([[You brought Dr. Mensing back from a Empire scientific conference.]]))
        misn.finish(true)
    end
end
