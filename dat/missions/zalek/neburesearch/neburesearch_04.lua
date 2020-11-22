--[[
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

require "dat/scripts/numstring.lua"
require "dat/missions/zalek/common.lua"

bar_desc = _("She probably has a new poorly paid job for you. Maybe she won't notice you if you leave now.")
mtitle = _("Shielding Prototype Funding")
mdesc = _("Help Dr. Mensing to get funding for constructing a shielding prototype.")
misn_reward = _("%s")

proposal_text = _([["It appears we keep running into each other %s. You may be happy to hear that I finished my theoretical work on the nebula resistant shielding technique. Of course I can't provide you a shielding system; we scientists usually don't bother with engineering. However in this case, I'd actually like to build a prototype shielding device. The prospect of exploring the Sol system is far too tempting.
    "This is were you come into play. I need a few capable engineers and some expensive hardware; my budget is too small though. This is why I have to acquire additional funding. Your task will be to chauffeur me around. Apparently it is sometimes required to show up in person. So annoying..."]])
accept_text = _([["Great! Our first destination is %s in the %s system."]])
decline_title = _("No science for today")
decline_text = _([["Too bad. Maybe you will change your mind."]])

setback_title = _("No luck")
first_planet_text = _([[After landing on %s Dr. Mensing tells you to wait until she returns. "Not more than a couple of periods." she said; in fact you had to wait for only two periods until she returned. She comes back looking defeated.
    "This is the first time that one of my applications was rejected. That's weird, I got positive feedback at first. It makes no sense that my application was rejected at the last minute. I guess things like this happen. Let's just go to %s in the %s system next and try again."]])
second_planet_text = _([["Alright, I'm sure it will work out this time!", Dr. Mensing said on arriving on %s. This time you have to wait even longer for her to return. The result is the same as her first try.
    "I don't get it. My presentation is flawless and my proposal is exciting. Why wouldn't they grant me additional funds? I tell you, something is wrong here! Hmm... Time to change tactics. I have to speak with Professor Voges himself but he is currently on %s in the %s system and just ignores us. I guess we have to go there to speak with him face-to-face."]])
ruadan_title = _("No clearance to land")
ruadan_text = _([[Apparently you are not allowed to land on %s and explaining the situation was futile. Dr. Mensing enters the cockpit asking why you aren't landing. "We're not allowed to? Let me try to talk with them."
    After a heated discussion Dr. Mensing gives up. "Right, they won't allow anyone to land on %s. That's so frustrating. Let's land on %s instead."]])
third_planet_text = _([["Good news! I asked around and found a clue how to contact Professor Voges. He promised one of his colleagues to show up to his party. Something about his wife, like they have gotten married or she died or something. Anyway, I managed to get invited there as well. So let's go to %s in the %s system!"]])
party_title = _("Crashing a party")
party_arrival_text = _([[The two of you arrive at the party site. "Listen, this is probably your first party so let me briefly explain to you how it works. Your goal usually is to talk with all people, draw some attention, let them know you were attending the party. Efficiency is most important here. The earlier you're done with demonstrating your presence and making a good impression; the earlier you can leave and do something useful.
    "You'll need an escape strategy to leave unnoticed though; It'd be rude when someone sees you leaving early. As you're a beginner I give you a tip: take the window of the bath room in the ground floor. No one will be able to see you from inside the house if you climb over the fence."]])
party_professor_text = _([[You wonder if anything of that was meant serious. Before you can ask, Dr. Mensing runs off engaging an older looking man, probably Professor Voges. She does not waste any time and brings up the issue straight on, visibly discomforting him. It's time to explore the party.
    You expect a Za'lek party to be much different from a regular party. In fact the first two things you notice are the classical music and the absence of any alcoholic drinks. There is however a buffet, in particular there is a small fridge with ice cream. You can spot a fair amount of people eating ice cream. Is this the Za'lek equivalent of beer?
    Before you know it you've engaged in a conversation with one of the scientists; He saw you enter with Dr. Mensing which motivates him to ask the question of the hour.. whether you and her are dating. In an attempt to clear his misgivings about the situation you tell him that she hired you. Hopefully he believes you..
    You continue your tour, as you come close to one of the windows from the corner of your eye you see something fall outside, followed by a dull thump. Looking out of the window you see a figure dressed in a lab coat trying to stand up slowly. You're about to open the window and ask if he needs help, but he notices you and gestures to be silent as he hobbles off. Those Za'lek are indeed trying to escape the party.]])
party_end_text = _([["At least the ice cream is great!" you think as you take another bite. Suddenly someone grabs your arm. "We're done here. Bring me back to %s!" Dr. Mensing quietly tells you. For a moment you what she is doing as she drags you towards the bathroom; finally you remember what she said about her 'escape strategy'. She mentioned that you are supposed to leave the building through the window of the bathroom.]])
home_title = _("Finally back home")
return_text = _([["That took long enough! I'm glad Professor Voges promised to take care of the funding. The problem was that my recent research is related to a secret project on Ruadan Prime and my funding was shut down - like some kind of conspiracy; can you believe it! Actually I'm not supposed to tell you anything as you could possibly get into a lot of trouble.. forget I said anything! I have much work to do!"
    She gives you a credit chip worth %s and leaves.]])

warning_title = _("Caution!")
warning_text = _([[You receive a system wide broadcast. It appears to be a warning. "Caution! A drone squadron in the %s system went haywire! Proceed with care."
    Why is this happening so frequently in Za'lek space? On a second glance you see on your radar that a drone squadron is flying right towards your ship.]])
second_warning_text = _([[The now familiar warning appears once again on your ship's screen. "Caution! A drone squadron in the %s system went haywire! Proceed with care."
    Just how often is this going to happen?]])

osd_msg   = {}
osd_title = _("Shielding Prototype Funding")
osd_fly_to = _("Land on %s in the %s system.")
osd_return = _("Return to %s in the %s system.")

log_text = _([[You helped Dr. Mensing to acquire funding for a shielding prototype that will enable to explore the Sol nebula.]])

function create()
    -- mission variables
    credits = 500000  -- 500K
    homeworld, homeworld_sys = planet.get("Jorla")
    dest_planet, dest_sys = planet.get("Jurai")
    
    -- Spaceport bar stuff
    misn.setNPC(_("Dr. Mensing"), "zalek/unique/mensing")
    misn.setDesc(bar_desc)
end

function accept()
    if not tk.yesno("", proposal_text:format(player:name())) then
        tk.msg(decline_title, decline_text)
        misn.finish()
    end
    tk.msg("", accept_text:format(_(dest_planet:name()), _(dest_sys:name())))
    stage = 0
    
    -- Set up mission information
    misn.setTitle(mtitle)
    misn.setReward(misn_reward:format(creditstring(credits)))
    misn.setDesc(mdesc:format(_(dest_planet:name()), _(dest_sys:name())))
    misn_marker = misn.markerAdd(dest_sys, "low")
    
    misn.accept()
    osd_msg[1] = osd_fly_to:format(dest_planet:name(), dest_sys:name())
    osd_msg[2] = osd_return:format(homeworld:name(), homeworld_sys:name())
    misn.osdCreate(osd_title, osd_msg)
    
    hook.land("land")
    hook.jumpin("jumpin")
    hook.takeoff("takeoff")
end

function land()
    landed = planet.cur()
    if landed == dest_planet then
        if stage == 0 then
            local planet_name = dest_planet:name()
            stage = 1
            dest_planet, dest_sys = planet.get("Neo Pomerania")
            tk.msg(setback_title, first_planet_text:format(planet_name, _(dest_planet:name()), _(dest_sys:name())))
            misn.markerMove(misn_marker, dest_sys)
            osd_msg[1] = osd_fly_to:format(dest_planet:name(), dest_sys:name())
            osd_msg[2] = osd_return:format(homeworld:name(), homeworld_sys:name())
            misn.osdCreate(osd_title, osd_msg)
        elseif stage == 2 then
            local planet_name = dest_planet:name()
            stage = 3
            dest_planet, dest_sys = planet.get("Ruadan Prime")
            tk.msg(setback_title, second_planet_text:format(planet_name, _(dest_planet:name()), _(dest_sys:name())))
            misn.markerMove(misn_marker, dest_sys)
            osd_msg[1] = osd_fly_to:format(dest_planet:name(), dest_sys:name())
            osd_msg[2] = osd_return:format(homeworld:name(), homeworld_sys:name())
            misn.osdCreate(osd_title, osd_msg)
        elseif stage == 4 then
            stage = 5
            dest_planet, dest_sys = planet.get("Excelcior")
            tk.msg("", third_planet_text:format(dest_planet:name(), dest_sys:name()))
            misn.markerMove(misn_marker, dest_sys)
            osd_msg[1] = osd_fly_to:format(dest_planet:name(), dest_sys:name())
            osd_msg[2] = osd_return:format(homeworld:name(), homeworld_sys:name())
            misn.osdCreate(osd_title, osd_msg)
        elseif stage == 5 or stage == 6 then
            stage = 7
            dest_planet = homeworld
            dest_sys = homeworld_sys
            tk.msg(party_title, party_arrival_text)
            tk.msg(party_title, party_professor_text)
            tk.msg(party_title, party_end_text:format(_(dest_planet:name())))
            misn.markerMove(misn_marker, dest_sys)
            misn.osdActive(2)
        elseif stage == 7 then
            tk.msg(home_title, return_text:format(creditstring(credits)))
            player.pay(credits)
            misn.markerRm(misn_marker)
            zlk_addNebuResearchLog(log_text)
            misn.finish(true)
        end
    end
end

function jumpin()
    if stage == 1 and system.cur() == dest_sys then
        local scom = {}
        stage = 2
        scom[1] = pilot.addRaw("Za'lek Light Drone", "mercenary", dest_planet, "Mercenary")
        scom[2] = pilot.addRaw("Za'lek Light Drone", "mercenary", dest_planet, "Mercenary")
        scom[3] = pilot.addRaw("Za'lek Heavy Drone", "mercenary", dest_planet, "Mercenary")
        for i=1,#scom do
            scom[i]:setHostile(false)
            scom[i]:control()
            scom[i]:attack(player.pilot())
        end
        hook.timer(10000, "warningMessage")
    elseif stage == 3 and system.cur() == dest_sys then
        hook.timer(60000, "cannotLand")
    end
end

function takeoff()
    if stage == 5 then
        stage = 6
        hook.timer(2000, "startAmbush")
        hook.timer(12000, "secondWarningMessage")
    end
end

function warningMessage()
    tk.msg(warning_title, warning_text:format(_(system.cur():name())))
end

function secondWarningMessage()
    tk.msg(warning_title, second_warning_text:format(_(system.cur():name())))
end

function startAmbush()
    local scom = {}
    local origins = {}
    origins[1] = system.get("Vauban")
    origins[2] = system.get("Woreck")
    origins[3] = system.get("Damien")
    for i=1,#origins do
        scom[2*i-1] = pilot.addRaw("Za'lek Light Drone", "mercenary", origins[i], "Mercenary")
        scom[2*i] = pilot.addRaw("Za'lek Heavy Drone", "mercenary", origins[i], "Mercenary")
    end
    for i=1,#scom do
        scom[i]:setHostile(false)
        scom[i]:control()
        scom[i]:attack(player.pilot())
    end
end

function cannotLand()
    local planet_name = dest_planet:name()
    stage = 4
    dest_planet, dest_sys = planet.get("Ruadan Station")
    tk.msg(ruadan_title, ruadan_text:format(planet_name, planet_name, _(dest_planet:name())))
    osd_msg[1] = osd_fly_to:format(dest_planet:name(), dest_sys:name())
    osd_msg[2] = osd_return:format(homeworld:name(), homeworld_sys:name())
    misn.osdCreate(osd_title, osd_msg)
end
