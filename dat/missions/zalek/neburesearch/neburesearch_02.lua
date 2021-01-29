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
</mission>
--]]
--[[
   
   Mission: Emergency of Immediate Inspiration
   
   Description: Take Dr. Mensing to Jorla as fast as possible!
   
   Difficulty: Easy

--]]

require "scripts/cargo_common"
require "scripts/numstring"
require "missions/zalek/common"


bar_desc = _("It appears she wants to talk with you.")
mtitle = _("Emergency of Immediate Inspiration")
misn_reward = _("%s")
mdesc = _("Take Dr. Mensing to %s in the %s system as fast as possible!")
bar_title = _("Bar")
landing_title = _("Mission accomplished")
bar_text = _([["Well met, %s! In fact, it's a lucky coincidence that we meet. You see, I'm in dire need of your service. I'm here on a... conference of sorts, not a real one. We are obligated to present the newest results of our research to scientists of the Empire once per period - even though these jokers lack the skills to understand our works! It's just a pointless ritual anyway. But I just got an ingenious idea on how to prevent the volatile Sol nebula from disrupting ship shields! I will spare you with the details - to ensure my idea is not going to be stolen, nothing personal. You can never be sure who is listening."
    "Anyway, you have to take me back to my lab on %s in the %s system immediately! I'd also pay %s if necessary."]])
accept_text = _([["Splendid! I'd like to start with my work as soon as possible, so please hurry! Off to %s we go!"
    With that being said she drags you out of the bar. When realizing that she actually does not know on which landing pad your ship is parked she lets you loose and orders you to lead the way.]])
arrival_text = _([["Finally! I can't await getting started. Before I forget -" She hands you over a credit chip worth %s.
    %s]])
late_arrival_text = _([["That took long enough! I can't await getting started. I doubt you deserve full payment. I'll rather give you a reduced payment of %s for educational reasons." She hands you over a credit chip.
    %s]])
request_text = _([["There's actually another thing I've almost forgotten. I also have to attend another conference very soon on behalf of professor Voges who obviously is very busy with some project he would not tell me about. But I don't want to go there - my research is far too important! So could you instead bring Robert there? You remember the student you helped out recently? I'm sure he will do the presentation just fine! I'll tell him to meet you in the bar as soon as possible!"
    With that being said Dr. Mensing leaves you immediately without waiting for your answer. It appears you should head to the bar to meet up with the student.]])

-- Mission info stuff
osd_msg   = {}
osd_title = _("Emergency of Immediate Inspiration")
osd_msg[1] = _("Fly to %s in the %s system.")

log_text = _([[You brought Dr. Mensing back from a Empire scientific conference.]])


function create()
    -- mission variables
    credits = 400000  -- 400K
    homeworld, homeworld_sys = planet.get("Jorla")
    origin = planet.cur()
    origin_sys = system.cur()
    
    local numjumps = origin_sys:jumpDist(homeworld_sys, false)
    local traveldist = cargo_calculateDistance(origin_sys, origin:pos(), homeworld_sys, homeworld)
    local stuperpx   = 0.15
    local stuperjump = 10000
    local stupertakeoff = 10000
    local allowance  = traveldist * stuperpx + numjumps * stuperjump + stupertakeoff + 240 * numjumps
    timelimit  = time.get() + time.create(0, 0, allowance)
    
    -- Spaceport bar stuff
    misn.setNPC(_("Dr. Mensing"), "zalek/unique/mensing.png")
    misn.setDesc(bar_desc)
end

function accept()
    if not tk.yesno(bar_title, bar_text:format(player:name(), homeworld:name(), homeworld_sys:name(), creditstring(credits))) then
        misn.finish()
    end
    tk.msg(bar_title, accept_text:format(homeworld:name()))
    
    -- Set up mission information
    misn.setTitle(mtitle)
    misn.setReward(misn_reward:format(creditstring(credits)))
    misn.setDesc(mdesc:format(homeworld:name(), homeworld_sys:name()))
    misn_marker = misn.markerAdd(homeworld_sys, "low")
    
    misn.accept()
    osd_msg[1] = osd_msg[1]:format(homeworld:name(), homeworld_sys:name())
    misn.osdCreate(osd_title, osd_msg)
    
    hook.land("land")
end

function land()
    landed = planet.cur()
    if landed == homeworld then
        if timelimit < time.get() then
            tk.msg(landing_title, late_arrival_text:format(creditstring(credits / 2), request_text))
            player.pay(credits / 2)
        else
            tk.msg(landing_title, arrival_text:format(creditstring(credits), request_text))
            player.pay(credits)
        end
        misn.markerRm(misn_marker)
        zlk_addNebuResearchLog(log_text)
        misn.finish(true)
    end
end

