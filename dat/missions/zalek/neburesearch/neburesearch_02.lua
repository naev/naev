--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Emergency of Immediate Inspiration">
 <unique />
 <priority>4</priority>
 <done>Advanced Nebula Research</done>
 <chance>30</chance>
 <location>Bar</location>
 <faction>Empire</faction>
 <cond>
   if system.get("Gamma Polaris"):jumpDist() &gt; 3 then
      return false
   end
   local t = spob.cur():tags()
   if t.station then
      return false
   end
   return true
 </cond>
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

local fmt = require "format"
local nebu_research = require "common.nebu_research"
local vn = require 'vn'

local mensing_portrait = nebu_research.mensing.portrait


-- Mission constants
local credits = nebu_research.rewards.credits02
local homeworld, homeworld_sys = spob.getS("Jorla")

function create()
    -- mission variables
    mem.origin = spob.cur()
    mem.origin_sys = system.cur()

    -- Spaceport bar stuff
    misn.setNPC(_("Dr. Mensing"), mensing_portrait, _("It appears she wants to talk with you."))
end

function accept()
    local accepted = false
    vn.clear()
    vn.scene()
    local mensing = vn.newCharacter( nebu_research.vn_mensing() )
    vn.transition("fade")

    mensing(fmt.f(_([["Well met, {player}! In fact, it's a lucky coincidence that we meet. You see, I'm in dire need of your service."]]), {player=player.name()}))
    mensing(_([["I'm here on a... conference of sorts, not a real one. We are obligated to present the newest results of our research to scientists of the Empire once per period - even though these jokers lack the skills to understand our work! It's just a pointless ritual anyway."]]))
    mensing(_([["But I just got an ingenious idea on how to prevent the volatile Sol nebula from disrupting ship shields! I will spare you with the details - to ensure my idea is not going to be stolen, nothing personal. You can never be sure who is listening."]]))
    mensing(fmt.f(_([["Anyway, you have to take me back to my lab on {pnt} in the {sys} system immediately! I'd also pay {credits} if necessary."]]), {pnt=homeworld, sys=homeworld_sys, credits=fmt.credits(credits)}))
    vn.menu( {
        { _("Accept"), "accept" },
        { _("Decline"), "decline" },
    } )

    vn.label( "decline" )
    vn.na(_("You don't want to be involved again in a dangerous, poorly paid job so you decline and leave the bar."))
    vn.done()

    vn.label( "accept" )
    vn.func( function () accepted = true end )
    mensing(_([["Splendid! I'd like to start with my work as soon as possible.
But before I forget, there's some issue..."]]))
    mensing(_([["You see, I'm not allowed to leave officially. Therefore I'd rather let them think that I was kidnapped. I'm sure it'll be fine! But don't let an Empire ship scan your ship! I don't know how they would react finding me onboard of your ship. Try to be stealthy and once we're in Za'lek territory there will be no problem."]]))
    vn.menu( {
        { fmt.f(_("Take her to {sys}"), {sys=homeworld_sys}), "accept" },
        { _("Leave her"), "decline" },
    } )
    vn.label( "decline" )
    vn.func( function () accepted = false end )
    vn.na(_("That sounds too risky for you. You'll probably end up dead or in prison."))
    vn.done()
    vn.label( "accept" )
    vn.func( function () accepted = true end )
    vn.run()
    if not accepted then
        misn.finish()
        return
    end

    -- Set up mission information
    misn.setTitle(_("Emergency of Immediate Inspiration"))
    misn.setReward(credits)
    misn.setDesc(fmt.f(_("Take Dr. Mensing to {pnt} in the {sys} system as fast as possible!"), {pnt=homeworld, sys=homeworld_sys}))
    mem.misn_marker = misn.markerAdd(homeworld, "low")

    local c = commodity.new( N_("Dr. Mensing"), N_("You need to bring Dr. Mensing to her destination but the Empire will assume you have kidnapped her if they scan you!") )
    c:illegalto( {"Empire"} )
    mem.carg_id = misn.cargoAdd( c, 0 )

    misn.accept()
    misn.osdCreate(_("Emergency of Immediate Inspiration"), {
       fmt.f(_("Fly to {pnt} in the {sys} system"), {pnt=homeworld, sys=homeworld_sys}),
    })

    hook.land("land")
end

function land()
    mem.landed = spob.cur()
    if mem.landed == homeworld then
        vn.clear()
        vn.scene()
        local mensing = vn.newCharacter( nebu_research.vn_mensing() )
        vn.transition("fade")
        mensing(fmt.f(_([["Finally! I can't await getting started. Before I forget -" She hands you a credit chip worth {credits}.]]), {credits=fmt.credits(credits)}))
        mensing(_([["There's actually another thing I've almost forgotten. I also have to attend another conference very soon on behalf of professor Voges who obviously is very busy with some project he would not tell me about. But I don't want to go there - my research is far too important! So could you instead bring Robert there? You remember the student you helped out recently? I'm sure he will do the presentation just fine! I'll tell him to meet you in the bar as soon as possible!"]]))
        mensing(_("With that being said, Dr. Mensing leaves you immediately without waiting for your answer. It appears you should head to the bar to meet up with the student."))
        vn.done()
        vn.run()
        player.pay(credits)
        misn.markerRm(mem.misn_marker)
        nebu_research.log(_([[You brought Dr. Mensing back from a Empire scientific conference.]]))
        misn.finish(true)
    end
end
