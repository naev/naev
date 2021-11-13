--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Shielding Prototype Funding">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <done>The Substitute Speaker</done>
  <chance>10</chance>
  <location>Bar</location>
  <faction>Za'lek</faction>
 </avail>
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
local zlk = require "common.zalek"

-- Mission constants
local credits = 500e3
local homeworld, homeworld_sys = planet.getS("Jorla")
local dest_planet, dest_sys = planet.getS("Jurai")

function create()
    misn.setNPC(_("Dr. Mensing"), "zalek/unique/mensing.webp", _("She probably has a new poorly paid job for you. Maybe she won't notice you if you leave now."))
end

function accept()
    if not tk.yesno("", fmt.f(_([["It appears we keep running into each other {player}. You may be happy to hear that I finished my theoretical work on the nebula resistant shielding technique. Of course I can't provide you a shielding system; we scientists usually don't bother with engineering. However in this case, I'd actually like to build a prototype shielding device. The prospect of exploring the Sol system is far too tempting.
    "This is were you come into play. I need a few capable engineers and some expensive hardware; my budget is too small though. This is why I have to acquire additional funding. Your task will be to chauffeur me around. Apparently it is sometimes required to show up in person. So annoying..."]]), {player=player:name()})) then
        tk.msg(_("No science for today"), _([["Too bad. Maybe you will change your mind."]]))
        misn.finish()
    end
    tk.msg("", fmt.f(_([["Great! Our first destination is {pnt} in the {sys} system."]]), {pnt=dest_planet, sys=dest_sys}))
    mem.stage = 0

    -- Set up mission information
    misn.setTitle(_("Shielding Prototype Funding"))
    misn.setReward(fmt.credits(credits))
    misn.setDesc(_("Help Dr. Mensing to get funding for constructing a shielding prototype."))
    mem.misn_marker = misn.markerAdd(dest_sys, "low")

    misn.accept()
    misn.osdCreate(_("Shielding Prototype Funding"), {
        fmt.f(_("Land on {pnt} in the {sys} system."), {pnt=dest_planet, sys=dest_sys}),
        fmt.f(_("Return to {pnt} in the {pnt} system."), {pnt=homeworld, hsys=omeworld_sys}),
    })

    hook.land("land")
    hook.jumpin("jumpin")
    hook.takeoff("takeoff")
end

local function dest_updated()
    misn.markerMove(mem.misn_marker, dest_sys)
    misn.osdCreate(_("Shielding Prototype Funding"), {
        fmt.f(_("Land on {pnt} in the {sys} system."), {pnt=dest_planet, sys=dest_sys}),
        fmt.f(_("Return to {pnt} in the {sys} system."), {pnt=homeworld, sys=homeworld_sys}),
    })
end

function land()
    mem.landed = planet.cur()
    if mem.landed == dest_planet then
        if mem.stage == 0 then
            mem.stage = 1
            dest_planet, dest_sys = planet.getS("Neo Pomerania")
            tk.msg(_("No luck"), fmt.f(_([[After landing on {cur_pnt} Dr. Mensing tells you to wait until she returns. "Not more than a couple of periods." she said; in fact you had to wait for only two periods until she returned. She comes back looking defeated.
    "This is the first time that one of my applications was rejected. That's weird, I got positive feedback at first. It makes no sense that my application was rejected at the last minute. I guess things like this happen. Let's just go to {pnt} in the {sys} system next and try again."]]), {cur_pnt=mem.landed, pnt=dest_planet, sys=dest_sys}))
            dest_updated()
        elseif mem.stage == 2 then
            mem.stage = 3
            dest_planet, dest_sys = planet.getS("Ruadan Prime")
            tk.msg(_("No luck"), fmt.f(_([["Alright, I'm sure it will work out this time!", Dr. Mensing said on arriving on {cur_pnt}. This time you have to wait even longer for her to return. The result is the same as her first try.
    "I don't get it. My presentation is flawless and my proposal is exciting. Why wouldn't they grant me additional funds? I tell you, something is wrong here! Hmm... Time to change tactics. I have to speak with Professor Voges himself but he is currently on {pnt} in the {sys} system and just ignores us. I guess we have to go there to speak with him face-to-face."]]), {cur_pnt=mem.landed, pnt=dest_planet, sys=dest_sys}))
            dest_updated()
        elseif mem.stage == 4 then
            mem.stage = 5
            dest_planet, dest_sys = planet.getS("Excelcior")
            tk.msg("", fmt.f(_([["Good news! I asked around and found a clue how to contact Professor Voges. He promised one of his colleagues to show up to his party. Something about his wife, like they have gotten married or she died or something. Anyway, I managed to get invited there as well. So let's go to {pnt} in the {sys} system!"]]), {pnt=dest_planet, sys=dest_sys}))
            dest_updated()
        elseif mem.stage == 5 or mem.stage == 6 then
            mem.stage = 7
            dest_planet = homeworld
            dest_sys = homeworld_sys
            tk.msg(_("Crashing a party"), _([[The two of you arrive at the party site. "Listen, this is probably your first party so let me briefly explain to you how it works. Your goal usually is to talk with all people, draw some attention, let them know you were attending the party. Efficiency is most important here. The earlier you're done with demonstrating your presence and making a good impression; the earlier you can leave and do something useful.
    "You'll need an escape strategy to leave unnoticed though; It'd be rude when someone sees you leaving early. As you're a beginner I give you a tip: take the window of the bath room in the ground floor. No one will be able to see you from inside the house if you climb over the fence."]]))
            tk.msg(_("Crashing a party"), _([[You wonder if anything of that was meant serious. Before you can ask, Dr. Mensing runs off engaging an older looking man, probably Professor Voges. She does not waste any time and brings up the issue straight on, visibly discomforting him. It's time to explore the party.
    You expect a Za'lek party to be much different from a regular party. In fact the first two things you notice are the classical music and the absence of any alcoholic drinks. There is however a buffet, in particular there is a small fridge with ice cream. You can spot a fair amount of people eating ice cream. Is this the Za'lek equivalent of beer?
    Before you know it you've engaged in a conversation with one of the scientists; He saw you enter with Dr. Mensing which motivates him to ask the question of the hour.. whether you and her are dating. In an attempt to clear his misgivings about the situation you tell him that she hired you. Hopefully he believes you..
    You continue your tour, as you come close to one of the windows from the corner of your eye you see something fall outside, followed by a dull thump. Looking out of the window you see a figure dressed in a lab coat trying to stand up slowly. You're about to open the window and ask if he needs help, but he notices you and gestures to be silent as he hobbles off. Those Za'lek are indeed trying to escape the party.]]))
            tk.msg(_("Crashing a party"), fmt.f(_([["At least the ice cream is great!" you think as you take another bite. Suddenly someone grabs your arm. "We're done here. Bring me back to {pnt}!" Dr. Mensing quietly tells you. For a moment you wonder what she is doing as she drags you towards the bathroom; finally you remember what she said about her 'escape strategy'. She mentioned that you are supposed to leave the building through the window of the bathroom.]]), {pnt=dest_planet}))
            misn.markerMove(mem.misn_marker, dest_sys)
            misn.osdActive(2)
        elseif mem.stage == 7 then
            tk.msg(_("Finally back home"), fmt.f(_([["That took long enough! I'm glad Professor Voges promised to take care of the funding. The problem was that my recent research is related to a secret project on Ruadan Prime and my funding was shut down - like some kind of conspiracy; can you believe it! Actually I'm not supposed to tell you anything as you could possibly get into a lot of trouble.. forget I said anything! I have much work to do!"
    She gives you a credit chip worth {credits} and leaves.]]), {credits=fmt.credits(credits)}))
            player.pay(credits)
            misn.markerRm(mem.misn_marker)
            zlk.addNebuResearchLog(_([[You helped Dr. Mensing to acquire funding for a shielding prototype that will enable to explore the Sol nebula.]]))
            misn.finish(true)
        end
    end
end

function jumpin()
    if mem.stage == 1 and system.cur() == dest_sys then
        local scom = {}
        mem.stage = 2
        scom[1] = pilot.add("Za'lek Light Drone", "Mercenary", dest_planet)
        scom[2] = pilot.add("Za'lek Light Drone", "Mercenary", dest_planet)
        scom[3] = pilot.add("Za'lek Heavy Drone", "Mercenary", dest_planet)
        for i=1,#scom do
            scom[i]:setHostile(false)
            scom[i]:control()
            scom[i]:attack(player.pilot())
        end
        hook.timer(10.0, "warningMessage")
    elseif mem.stage == 3 and system.cur() == dest_sys then
        hook.timer(60.0, "cannotLand")
    end
end

function takeoff()
    if mem.stage == 5 then
        mem.stage = 6
        hook.timer(2.0, "startAmbush")
        hook.timer(12.0, "secondWarningMessage")
    end
end

function warningMessage()
    tk.msg(_("Caution!"), fmt.f(_([[You receive a system wide broadcast. It appears to be a warning. "Caution! A drone squadron in the {sys} system went haywire! Proceed with care."
    Why is this happening so frequently in Za'lek space? On a second glance you see on your radar that a drone squadron is flying right towards your ship.]]), {sys=system.cur()}))
end

function secondWarningMessage()
    tk.msg(_("Caution!"), fmt.f(_([[The now familiar warning appears once again on your ship's screen. "Caution! A drone squadron in the {sys} system went haywire! Proceed with care."
    Just how often is this going to happen?]]), {sys=system.cur()}))
end

function startAmbush()
    local scom = {}
    local origins = {}
    origins[1] = system.get("Vauban")
    origins[2] = system.get("Woreck")
    origins[3] = system.get("Damien")
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
    local cur_planet = dest_planet
    mem.stage = 4
    dest_planet, dest_sys = planet.getS("Ruadan Station")
    tk.msg(_("No clearance to land"), fmt.f(_([[Apparently you are not allowed to land on {cur_pnt} and explaining the situation was futile. Dr. Mensing enters the cockpit asking why you aren't landing. "We're not allowed to? Let me try to talk with them."
    After a heated discussion Dr. Mensing gives up. "Right, they won't allow anyone to land on {cur_pnt}. That's so frustrating. Let's land on {pnt} instead."]]), {cur_pnt=cur_planet, pnt=dest_planet}))
    dest_updated()
end
