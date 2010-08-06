--[
-- This is the third mission in the "shadow" series.
--]]

include ("scripts/proximity.lua")
include ("scripts/chatter.lua")

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english

    title = {}
    text = {}
    
    title[1] = "An urgent invitation"
    text[1] = [[    Suddenly, out of nowhere, one of the dormant panels in your cockpit springs to life. It shows you a face you've never seen before in your life, but you recognize the plain grey uniform as belonging to the Four Winds.
    "Hello %s," the face says. "You must be wondering who I am and how it is I'm talking to you like this. Neither question is important. What is important is that captain Rebina has urgent need of your services. You are to meet her on the Seiryuu, which is currently in orbit around %s in the % system. Please don't ask any questions now. We expect to see you as quickly as you can make your way here."
    The screen goes dead again. You decide to make a note of this in your log. Perhaps it would be a good idea to visit the Seiryuu once more, if only to find out how they got a private line to your ship!]]
    
    title[2] = ""
    text[2] = [[    ]]
    
    -- Mission info stuff
    osd_title = {}
    osd_msg   = {}
    osd_title = "Dark Shadow"
    osd_msg[1] = ""
    
    misn_desc1 = [[You have been summoned to the %s system, where the Seiryuu is supposedly waiting for you in orbit around %s.]]
    misn_desc2 = [[You have been tasked by captain Rebina of the Four Winds to assist Jorek McArthy.]]
    misn_reward = "A sum of money."
end

function create()
    var.push("darkshadow_active")
    
    seirplanet, seirsys = planet.get("Edergast")
    
    tk.msg(title[1], text[1]:format(player.name(), seirplanet:name(), seirsys:name())
    accept() -- The player automatically accepts this mission. If he doesn't want it, he'll have to manually abort.
end

-- This is the initial phase of the mission, when it still only shows up in the mission list. No OSD, reward or markers yet.
function accept()
    misn.setDesc(misn_desc1:format(seirsys:name(), seirplanet:name()))
    misn.accept()

    stage = 1

    hook.jumpin("jumpin") 
end

-- This is the "real" start of the mission. Get yer mission variables here!
function accept2()
    misn.setDesc(misn_desc2)
    misn.setReward(misn_reward)
    misn.osdCreate(osd_title, osd_msg) 
end

-- Jumpin hook
function jumpin()
    if system.cur() == seirsys then
        seiryuu = pilot.add("Seiryuu", nil, vec2.new(300, 300) + seirplanet:pos())[1]
        seiryuu:setInvincible(true)
        seiryuu:disable()
        if stage == 1 or stage == 3 then
            hook.pilot(seiryuu, "board", "board")
        else
            seiryuu:setNoboard(true)
        end
    end
end

-- Handle boarding of the Seiryuu
function board()
    if stage == 1 then -- Briefing
        accept2()
        stage = 2
    elseif stage == 3 then -- Debriefing
        var.pop("darkshadow_active") 
        misn.finish(true)
    end
end

-- Handle the unsuccessful end of the mission.
function abort()
    var.pop("darkshadow_active") 
    misn.finish(false)
end
