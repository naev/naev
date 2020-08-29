--[[
<mission name="Novice Nebula Research">
 <lua>zalek/neburesearch/neburesearch_00</lua>
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <location>Bar</location>
  <planet>Jorla</planet>
 </avail>
</mission>
--]]
--[[
   
   Mission: Novice Nebula Research
   
   Description: A Za'lek student asks the player to help him with his reasearch. The player has to visit Doeston and Iris.
   Some minor complications on the way have to be expected.
   
   Difficulty: Easy

]]--

include "dat/scripts/numstring.lua"


t_sys = {}
t_sys[1] = "Doeston"
t_sys[2] = "Iris"

-- localization stuff, translators would work here
bar_desc = _("You see a young scientist talking with some pilots, apparently without success.")
mtitle = {}
mtitle[1] = _("Novice Nebula Research")
misn_reward = _("%s and the gratitude of a student")
mdesc = {}
mdesc[1] = _("Go to the %s system.")
title = {}
title[1] = _("Bar")
title[2] = _("Please...")
title[3] = _("What a mess...")
title[4] = _("Ready")
title[5] = _("Technical problems")
title[6] = _("Measurements completed")
title[7] = _("Mission Success")
text = {}
text[1] = _([["Hello Captain! You are a pilot, right? For my project I require a ship heading to the Sol Nebula. Certainly you must be interested in the proposal of researching the phenomenon that cut us off from mankind's patrimony.
    As this is the point where any other pilots I asked backed out I should start with my problem; due to some unfortunate circumstances the payment for this mission will be only %s. But rest assured, you will be mentioned in the acknowledgment section of my next paper!"]])
text[2] = _([["Hold up! Look, the problem is that my grant was not permitted to the extent that I asked for. Those idiots just don't understand the relevance of my research and cut my funds. Only because I'm still a student they completely underestimate my abilities -- by far!
    Now I spent all my credits on this sensor suit without the possibility to use it. You must know how this feels like. I mean, your ship is obviously in a very bad shape. So why don't you just help me out here?"]])
text[3] = _([["You do not have enough free cargo space to accept this mission!"]])
text[4] = _([["Great! I'll start to load the sensors into your ship right away. We should be ready to take off soon."]])
text[5] = _([["So it is not a problem at all? I'm still a student and spent all funds I got on the sensor suit. Thank you for helping me out here! I'll start to load the sensors into your ship right away. We should be ready to take off soon."]])
text[6] = _([[As you enter your ship you notice dozens of cables of various colors stretched across your ship's corridors. It is a complete mess. Even weirder is the fact that it is less than one period ago that you met the student in the bar. What has he done to your ship? You follow the direction where most of the cable seem to lead to. Finally you find the culprit.]])
text[7] = _([["Oh, hello again, Captain! I'm done just by now here, so we can take off whenever you're ready. I have to calibrate the sensors during the flight so there is no need to rush. Our first destination is %s."
    After asking what he has done to your ship he just answers, "Why, I just installed the sensors as appointed. It should have no unwanted side effects on your ship. Just a complete mess? Haven't you noticed the color coding? Well, I have my own system so it may appears confusing to you. Don't worry, I know exactly what I'm doing!"
    His last words are supposed to be reassuring but instead you realize accepting this mission was not the best idea...]])
text[9] = _([[After reaching the %s system the student enters your cockpit. As he realized he forgot to even tell you his name he introduces himself as Robert Hofer and claims to be a student of professor Voges himself. You haven't ever heard of that name but it sounds like he is famous within House Za'lek.
    "I will now start with the measurements. The density of the nebula is lower in this sector so that it is less volatile. For the real measurements we have to enter %s. I will tell you once we're done here."]])
text[10] = _([[Suddenly you lose control over your ship. Apparently most core systems were shut down. Something drains your ship's energy and there are black outs in several parts of your ship.
    You realize that your shields are down as well. In an environment like this... That's it, your going to die here! You knew accepting this mission was a mistake from the very first moment.]])
text[11] = _([[It seems you are still alive and all core components are back online!
    There is a certain someone you should have a serious talk to.]])
text[12] = _([["Sorry for causing trouble. I'm not quite familiar with the electronics of this ship type. You really should fly a Za'lek ship instead. It wouldn't have possibly happened!"
    After telling him that your ship was almost destroyed he just replies "That's interesting! So the nebula is corrosive as well? I should investigate the damage it caused to the armour once we landed. But first bring us to the %s system! Don't worry the blackout will not occur again!"
    It would probably be the best to just drop him off on the next planet...]])
text[13] = _([["Thank you for your trust! I will try to make it quick. No, wait, I'll rather try to make it without another blackout. Haha!"
    You start to think it was a big mistake to come to the %s system. "Anyway, just fly a circle or something. I will tell you once the scan is complete."]])
text[14] = _([["And that's it! Now, let's head back to %s."]])
text[15] = _([[The student has already started to remove all the cables and sensors inside your ship during the flight back to %s. When you land everything is packed into a couple of crates.
    "Once again, thank you for your help. I still have to analyze the data but it looks promising so far. With these results no one is going to question my theories anymore! Also, I decided to increase your reward due to the trouble I caused."
    With these words he gives you a credit chip worth %s and heads off. More worth than the money though is the insight that working for the Za'lek will be dangerous and tiresome.]])
choice1 = _("Ask")
choice2 = _("Nevermind")

-- Mission info stuff
osd_msg   = {}
osd_title = _("Novice Nebula Research")
osd_msg[1] = _("Fly to the %s system.")
osd_msg[2] = _("Fly to the %s system.")
osd_msg[3] = _("Return to the %s system.")

log_text = _([[You helped a Za'lek student to collect sensor data of the Sol nebula.]])


function create()
    -- mission variables
    misn_stage = 0
    homeworld = planet.get("Jorla")
    homeworld_sys = system.get("Regas")
    credits = 100000
    
    -- Spaceport bar stuff
    misn.setNPC(_("A young scientist"),  "zalek/unique/student")
    misn.setDesc(bar_desc)
end

function accept()
    -- Check for cargo space
    if player.pilot():cargoFree() <  5 then
        tk.msg(title[1], text[3])
        misn.finish()
    end
    
    if not tk.yesno(title[1], string.format(text[1], creditstring(50000))) then
        if not tk.yesno(title[2], text[2] ) then
            misn.finish()
        else
            tk.msg(title[1], string.format(text[4], t_sys[1]))
        end
    else
        tk.msg(title[1], string.format(text[5], t_sys[1]))
    end
    
    -- Add cargo
    cargo = misn.cargoAdd("Nebula Sensor Suit", 5)
    
    -- Set up mission information
    misn.setTitle(mtitle[1])
    misn.setReward(string.format(misn_reward, creditstring(50000)))
    misn.setDesc(string.format(mdesc[1], t_sys[1]))
    misn_marker = misn.markerAdd(system.get(t_sys[1]), "low")
    
    -- Add mission
    misn.accept()
    osd_msg[1] = string.format(osd_msg[1], t_sys[1])
    osd_msg[2] = string.format(osd_msg[2], t_sys[2])
    osd_msg[3] = string.format(osd_msg[3], homeworld_sys:name())
    misn.osdCreate(osd_title, osd_msg)
    
    thook = hook.takeoff("takeoff")
    hook.land("land")
    hook.enter("jumpin")
end

function land()
    landed = planet.cur()
    if misn_stage == 3 and landed == homeworld then
        tk.msg(title[7], string.format(text[15], homeworld:name(), creditstring(credits)))
        misn.cargoRm(cargo)
        player.pay(credits)
        misn.markerRm(misn_marker)
        zlk_addNebuResearchLog(log_text)
        misn.finish(true)
    end
end

function takeoff()
    tk.msg(title[3], text[6])
    tk.msg(title[3], string.format(text[7], t_sys[1]))
    hook.rm(thook)
end

function jumpin()
    sys = system.cur()
    if misn_stage == 0 and sys == system.get(t_sys[1]) then
        hook.timer(5000, "beginFirstScan")
    elseif misn_stage == 1 and sys == system.get(t_sys[2]) then
        misn_stage = 2
        hook.timer(5000, "beginSecondScan")
    end
end

function beginFirstScan()
    tk.msg(title[4], string.format(text[9], t_sys[1], t_sys[2]))
    shook = hook.timer(30000, "startProblems")
end

function startProblems()
    -- Cancel autonav.
    player.cinematics(true)
    player.cinematics(false)
    ps = player.pilot()
    ps:control()
    phook = hook.timer(100, "drainShields")
    hook.timer(4000, "noticeProblems")
end

function drainShields()
    ps = player.pilot()
    armour, shield, stress, dis = ps:health()
    ps:setHealth(armour, 0)
    ps:setEnergy(0)
    phook = hook.timer(100, "drainShields")
end

function noticeProblems()
    tk.msg(title[5], text[10])
    hook.timer(10000, "stopProblems")
end

function stopProblems()
    ps:control(false)
    ps:setEnergy(100)
    tk.msg(title[5], text[11])
    tk.msg(title[5], string.format(text[12], t_sys[2]))
    misn_stage = 1
    misn.setDesc(string.format(mdesc[1], t_sys[2]))
    misn.markerMove(misn_marker, system.get(t_sys[2]))
    misn.osdActive(2)
    hook.rm(phook)
    hook.enter("jumpin")
end

function beginSecondScan()
    tk.msg(title[4], string.format(text[13], t_sys[2]))
    hook.timer(30000, "endSecondScan")
end

function endSecondScan()
    tk.msg(title[6], string.format(text[14], homeworld:name()))
    misn_stage = 3
    misn.setDesc(string.format(mdesc[1], homeworld:name()))
    misn.markerMove(misn_marker, homeworld_sys)
    misn.osdActive(3)
end

