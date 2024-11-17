--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Shielding Prototype Funding">
 <unique />
 <priority>4</priority>
 <done>The Substitute Speaker</done>
 <chance>10</chance>
 <location>Bar</location>
 <faction>Za'lek</faction>
 <cond>system.get("Hideyoshi's Star"):jumpDist() &gt; 2</cond>
 <notes>
  <campaign>Nebula Research</campaign>
 </notes>
</mission>
--]]
--[[

   Mission: Shielding Prototype Funding

   Description: The player has to help acquiring funds for the shielding prototype. This is going to be hilariously complicated.

   Difficulty: Easy to Medium

--]]
local fmt = require "format"
local nebu_research = require "common.nebu_research"
local vn = require 'vn'

local mensing_portrait = nebu_research.mensing.portrait

-- Mission constants
local homeworld, homeworld_sys = spob.getS("Jorla")
local dest_planet, dest_sys = spob.getS("Jurai")
local credits = nebu_research.rewards.credits04

--[[
Stages
0: mission start
1: first land on Jurai done
2: jumped in and hostile drones spawned
3: landed on Jurai and head towards Ruadan
4: can't land message
5: landed and on Ruadan Terminal the way to Excelcior
6: (second ambush, can be skipped )
7: landed on Excelcior, on way back
--]]

function create()
    misn.setNPC(_("Dr. Mensing"), mensing_portrait, _("She probably has a new poorly-paid job for you. Maybe she won't notice you if you leave now."))
end

function accept()
    local accepted = false
    vn.clear()
    vn.scene()
    local mensing = vn.newCharacter( nebu_research.vn_mensing() )
    vn.transition("fade")

    mensing(fmt.f(_([["It appears we keep running into each other, {player}. You may be happy to hear that I finished my theoretical work on the nebula resistant shielding technique. Of course I can't provide you a shielding system; we scientists usually don't bother with engineering. However, in this case, I'd actually like to build a prototype shielding device. The prospect of exploring the Sol system is far too tempting."]]),{player=player.name()}))
    mensing(_([["This is were you come into play. I need a few capable engineers and some expensive hardware; my budget is too small though. This is why I have to acquire additional funding. Your task will be to chauffeur me around. Apparently it is sometimes required to show up in person. So annoying..."]]))
    vn.menu( {
        { _("Accept the job"), "accept" },
        { _("Decline to help"), "decline" },
    } )
    vn.label( "decline" )
    mensing(_([["Too bad. Maybe you will change your mind."]]))
    vn.done()
    vn.label( "accept" )
    mensing(fmt.f(_([["Great! Our first destination is {pnt} in the {sys} system."]]), {pnt=dest_planet, sys=dest_sys}))
    vn.func( function () accepted = true end )
    vn.done()
    vn.run()

    if not accepted then
        return
    end

    mem.stage = 0
    mem.dest_planet, mem.dest_sys = dest_planet, dest_sys

    -- Set up mission information
    misn.setTitle(_("Shielding Prototype Funding"))
    misn.setReward(credits)
    misn.setDesc(_("Help Dr. Mensing to get funding to construct a shielding prototype."))
    mem.misn_marker = misn.markerAdd(mem.dest_planet, "low")

    misn.accept()
    misn.osdCreate(_("Shielding Prototype Funding"), {
        fmt.f(_("Land on {pnt} in the {sys} system"), {pnt=dest_planet, sys=dest_sys}),
        fmt.f(_("Return to {pnt} in the {sys} system"), {pnt=homeworld, sys=homeworld_sys}),
    })

    hook.land("land")
    hook.jumpin("jumpin")
    hook.takeoff("takeoff")
end

local function dest_updated()
   misn.markerMove(mem.misn_marker, mem.dest_planet)
   misn.osdCreate(_("Shielding Prototype Funding"), {
      fmt.f(_("Land on {pnt} in the {sys} system"), {pnt=mem.dest_planet, sys=mem.dest_sys}),
      fmt.f(_("Return to {pnt} in the {sys} system"), {pnt=homeworld, sys=homeworld_sys}),
    })
end

function land()
    mem.landed = spob.cur()
    if mem.landed == mem.dest_planet then
        vn.clear()
        vn.scene()
        local mensing = vn.newCharacter( nebu_research.vn_mensing() )
        vn.transition("fade")
        if mem.stage == 0 then
            mem.stage = 1
            mem.dest_planet, mem.dest_sys = spob.getS("Neo Pomerania")
            vn.na(fmt.f(_([[After landing on {cur_pnt}, Dr. Mensing tells you to wait until she returns. "Not more than a couple of periods." she said; in fact you had to wait for only two periods until she returned. She comes back looking defeated.]]), {cur_pnt=mem.landed}))
            mensing(fmt.f(_([["This is the first time that one of my applications was rejected. That's weird. I got positive feedback at first. It makes no sense that my application was rejected at the last minute. I guess things like this happen. Let's just go to {pnt} in the {sys} system next and try again."]]), {pnt=mem.dest_planet, sys=mem.dest_sys}))
            vn.done()
            vn.run()
            dest_updated()
        elseif mem.stage == 2 then
            mem.stage = 3
            mem.dest_planet, mem.dest_sys = spob.getS("Ruadan Prime")
            mensing(fmt.f(_([["Alright, I'm sure it will work out this time!", Dr. Mensing said on arriving on {cur_pnt}.]]), {cur_pnt=mem.landed}))
            vn.na(_([[This time you have to wait even longer for her to return. The result is the same as her first try.]]))
            mensing(fmt.f(_([["I don't get it. My presentation is flawless and my proposal is exciting. Why wouldn't they grant me additional funds? I tell you, something is wrong here! Hmm... Time to change tactics. I have to speak with Professor Voges himself but he is currently on {pnt} in the {sys} system and just ignores us. I guess we have to go there to speak with him face-to-face."]]), {pnt=mem.dest_planet, sys=mem.dest_sys}))
            vn.done()
            vn.run()
            dest_updated()
        elseif mem.stage == 4 then
            mem.stage = 5
            mem.dest_planet, mem.dest_sys = spob.getS("Excelcior")
            mensing(fmt.f(_([["Good news! I asked around and found a way to contact Professor Voges. He promised to show up to one of his colleague's parties. Something about his wife, like they have gotten married, or she died, or something. Anyway, I managed to get invited there as well. So let's go to {pnt} in the {sys} system!"]]), {pnt=mem.dest_planet, sys=mem.dest_sys}))
            vn.done()
            vn.run()
            dest_updated()
        elseif mem.stage == 5 or mem.stage == 6 then
            mem.stage = 7
            mem.dest_planet, mem.dest_sys = homeworld, homeworld_sys
            vn.na(_([[The two of you arrive at the location of the party.]]))
            mensing(_([["Listen, this is probably your first party, so let me briefly explain to you how it works. Your goal, usually, is to talk with all people, draw some attention, let them know you attended the party. Efficiency is most important here. The earlier you're done with demonstrating your presence and making a good impression; the earlier you can leave and do something useful."]]))
            mensing(_([["You'll need an escape strategy to leave unnoticed though; It's gauche if someone sees you leaving early. As you're a beginner, I'll give you a tip: sneak out the window of the bathroom on the ground floor. No one will be able to see you from inside the house if you climb over the fence."]]))
            vn.na(_([[You wonder if Dr. Mensing meant any of that seriously. Before you can ask, Dr. Mensing runs off, engaging an older looking man, probably Professor Voges. She does not waste any time and brings the issue straight on, visibly discomforting him. It's time to explore the party.]]))
            vn.disappear( mensing, "slideleft" )
            vn.na(_([[You expect a Za'lek party to be much different from a regular party. In fact, the first two things you notice are the classical music and the absence of any alcoholic drinks. There is, however, a buffet. In particular, there is a small fridge with ice cream. You spot a fair amount of people eating ice cream. Is this the Za'lek equivalent of beer?]]))
            vn.na(_([[Before you know it, you've engaged in a conversation with one of the scientists; He saw you enter with Dr. Mensing, which motivates him to ask the question of the hour… whether you and her are dating. In an attempt to clear his misgivings about the situation you tell him that she hired you. Hopefully he believes you…]]))
            vn.na(_([[You continue your tour. As you come close to one of the windows from the corner of your eye you see something fall outside, followed by a dull thump. Looking out of the window you see a figure dressed in a lab coat trying to stand up slowly. You're about to open the window and ask if he needs help, but he notices you and gestures to be silent as he hobbles off. These Za'lek are indeed trying to escape the party.]]))
            vn.na(_([["At least the ice cream is great!" you think as you take another bite. Suddenly someone grabs your arm.]]))
            vn.appear( mensing, "slideleft" )
            mensing(fmt.f(_([["We're done here. Bring me back to {pnt}!" Dr. Mensing quietly tells you.]]), {pnt=mem.dest_planet}))
            vn.na(_([[For a moment you wonder what she is doing as she drags you towards the bathroom; finally you remember what she said about her 'escape strategy'. She mentioned that you are supposed to leave the building through the window of the bathroom.]]))
            vn.done()
            vn.run()
            misn.markerMove(mem.misn_marker, mem.dest_planet)
            misn.osdActive(2)
        elseif mem.stage == 7 then
            mensing(_([["That took long enough! I'm glad Professor Voges promised to take care of the funding. The problem was that my recent research is related to a secret project and my funding was shut down - like some kind of conspiracy; can you believe it! Actually I'm not supposed to tell you anything as you could possibly get into a lot of trouble.. forget I said anything! I have much work to do!"]]))
            vn.na(fmt.f(_([[She gives you a credit chip worth {credits} and leaves.]]), {credits=fmt.credits(credits)}))
            vn.done()
            vn.run()
            player.pay(credits)
            nebu_research.log(_([[You helped Dr. Mensing to acquire funding for a shielding prototype that will enable to explore the Sol nebula.]]))
            misn.finish(true)
        end
    end
end

function jumpin()
   if mem.stage == 1 and system.cur() == mem.dest_sys then
      local scom = {}
      mem.stage = 2
      scom[1] = pilot.add("Za'lek Light Drone", "Mercenary", mem.dest_planet, nil, {ai="baddiepos"})
      scom[2] = pilot.add("Za'lek Light Drone", "Mercenary", mem.dest_planet, nil, {ai="baddiepos"})
      scom[3] = pilot.add("Za'lek Heavy Drone", "Mercenary", mem.dest_planet, nil, {ai="baddiepos"})
      for k,p in ipairs(scom) do
         p:setHostile(true)
         p:memory().guardpos = player.pos() -- Just go towards the player position and attack if they are around
      end
      hook.timer(10.0, "warningMessage")
   elseif mem.stage == 3 and system.cur() == mem.dest_sys then
      hook.timer(60.0, "cannotLand")
   end
end

function takeoff()
   if mem.stage == 5 then
      mem.stage = 6
      hook.timer(2, "startAmbush")
      hook.timer(12, "secondWarningMessage")
   end
end

function warningMessage()
    tk.msg(_("Caution!"), fmt.f(_([[You receive a system wide broadcast. It appears to be a warning. "Caution! A drone squadron in the {sys} system went haywire! Proceed with care."
    Why is this happening so frequently in Za'lek space? A second glance at your radar shows that a drone squadron is flying right towards your ship.]]), {sys=system.cur()}))
end

function secondWarningMessage()
    tk.msg(_("Caution!"), fmt.f(_([[The now familiar warning appears once again on your ship's screen. "Caution! A drone squadron in the {sys} system went haywire! Proceed with care."
    Just how often is this going to happen?]]), {sys=system.cur()}))
end

function startAmbush()
   local scom = {}
   local origins = {}
   for k,j in ipairs(system.cur():jumps()) do
      table.insert( origins, j:dest() )
   end
   for i=1,#origins do
      scom[2*i-1] = pilot.add("Za'lek Light Drone", "Mercenary", origins[i])
      scom[2*i] = pilot.add("Za'lek Heavy Drone", "Mercenary", origins[i])
   end
   for i=1,#scom do
      scom[i]:setHostile(false)
      scom[i]:control()
      scom[i]:attack(player.pilot())
   end
end

function cannotLand()
    local cur_planet = mem.dest_planet
    mem.stage = 4
    mem.dest_planet, mem.dest_sys = spob.getS("Ruadan Terminal")
    vn.clear()
    vn.scene()
    local mensing = vn.newCharacter( nebu_research.vn_mensing() )
    vn.transition("fade")
    vn.na(fmt.f(_([[Apparently, you are not allowed to land on {cur_pnt} and explaining the situation was futile. Dr. Mensing enters the cockpit asking why you aren't landing.]]), {cur_pnt=cur_planet}))
    mensing(_([["We're not allowed to? Let me try to talk with them."]]))
    vn.na(_([[After a heated discussion Dr. Mensing gives up.]]))
    mensing(fmt.f(_([["Right, they won't allow anyone to land on {cur_pnt}. That's so frustrating. Let's land on {pnt} instead."]]), {cur_pnt=cur_planet, pnt=mem.dest_planet}))
    vn.done()
    vn.run()
    dest_updated()
end
