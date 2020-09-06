--[[
<mission name="Novice Nebula Research">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <location>Bar</location>
  <planet>Jorla</planet>
 </avail>
  <notes>
   <campaign>Nebula Research</campaign>
   <tier>2</tier>
  </notes>
</mission>
--]]
--[[
   
   Mission: Novice Nebula Research
   
   Description: A Za'lek student asks the player to help him with his reasearch. The player has to visit Doeston and Iris.
   Some minor complications on the way have to be expected.
   
   Difficulty: Easy

]]--

require "dat/scripts/numstring.lua"


text = {}
bar_ask_text = _([["Hello there! You are a pilot, right? For my project I require a ship that can go to the Nebula. Certainly you must be interested in the proposal of researching the phenomenon that cut us off from mankind's patrimony.
    "As this is the point where any other pilots I asked backed out, I should start by mentioning that due to some unfortunate circumstances the payment for this mission will be only %s. But rest assured, you will be mentioned in the acknowledgment section of my next paper!"]])
bar_ask_again_text = _([["Hold up! Look, the problem is that my grant was not permitted to the extent that I asked for. Those simpletons cut my funds because they just don't understand the relevance of my research. Just because I'm still a student they completely underestimate my abilities!
    "Now I've spent all my credits on this sensor suit without the ability to use it. You must know how this feels. I mean, your ship obviously could use some work. So why don't you just help me out here?"]])
takeoff_text = _([[As you enter your ship you notice dozens of cables of various colors stretched across your ship's corridors. It is a complete mess. You follow the direction most of the cables seem to lead to and find the culprit.
    "Oh, hello again, Captain! I'm done with my work here, so we can take off whenever you're ready. I have to calibrate the sensors during the flight so there is no need to rush. Our first destination is %s." You try to maintain composure as you ask him what he has done to your ship. "Oh, I just installed the sensors. It should have no unwanted side effects on your ship.
    "A mess, you say? Haven't you noticed the color coding? Don't worry, I know exactly what I'm doing!" His last words are supposed to be reassuring but instead you start to think that accepting this mission was not the best idea.]])
scan_1_text = _([[The student enters your cockpit as you arrive in the %s system. "Greetings, Captain! I realize I forgot to introduce myself. My name is Robert Hofer, student of Professor Voges himself! I'm sure you must have heard of him?" You tell him that the name doesn't sound familiar to you. "How can that be? Well, you would understand if you were a Za'lek.
    "Anyway, I will now start with the measurements. The density of the nebula is lower in this sector, so it's not particularly volatile. For the real measurements we have to enter %s. I will let you know when we're done here."]])
problems_text = _([[Suddenly you lose control of your ship. Apparently most core systems were shut down. Something drains your ship's energy and there are black outs in several parts of your ship.
    You realize that your shields are down as well. In an environment like this... That's it, your going to die here! You knew accepting this mission was a mistake from the very first moment.]])
problems_end_text = _([[You breathe a sigh of relief. It seems you're still alive. You try not to glare at Robert Hofer, but apparently aren't particularly successful considering his response. "Sorry for causing trouble. I'm not quite familiar with the electronics of this ship type. You really should fly a Za'lek ship instead. Those are so much better!
    "I should investigate the damage it caused to the armor once we land. But first we must go to the %s system. Don't worry, the blackout will not occur again!"]])
finish_text = _([[The student has already removed all the cables and sensors inside your ship during the flight back to %s. Everything is packed into a couple of crates by the time you land.
    "Once again, thank you for your help. I still have to analyze the data but it looks promising so far. With these results no one is going to question my theories anymore! Also, I decided to increase your reward to compensate for the trouble I caused."
    He gives you a credit chip worth %s and heads off. The money is nice, but not worth as much as the insight that working for the Za'lek will be dangerous and tiresome.]])


function create()
    -- mission variables
    misn_stage = 0
    t_sys = {}
    t_sys[1] = "Doeston"
    t_sys[2] = "Iris"
    homeworld, homeworld_sys = planet.get("Jorla")
    credits = 100000
    
    -- Spaceport bar stuff
    misn.setNPC(_("A young scientist"),  "zalek/unique/student")
    misn.setDesc( _("You see a young scientist talking with some pilots, apparently without success.") )
end

function accept()
    local bar_title = _("Science Needs You")

    -- Check for cargo space
    if player.pilot():cargoFree() <  5 then
        tk.msg( "", _([["Sorry, I need a ship with more cargo space than you have."]]) )
        misn.finish()
    end
    
    if not tk.yesno(bar_title, string.format(bar_ask_text, creditstring(50000))) then
        if not tk.yesno(bar_title, bar_ask_again_text ) then
            misn.finish()
        else
            tk.msg( bar_title, string.format(
                _([["Great! I'll start loading the sensors into your ship right away. We should be ready to take off soon."]]),
                t_sys[1] ) )
        end
    else
        tk.msg( bar_title, string.format(
            _([["So it is not a problem at all? I'm still a student and spent all funds I got on the sensor suit. Thank you for helping me out here! I'll start to load the sensors into your ship right away. We should be ready to take off soon."]]),
            t_sys[1] ) )
    end
    
    -- Add cargo
    cargo = misn.cargoAdd("Nebula Sensor Suit", 5)
    
    -- Set up mission information
    misn.setTitle( _("Novice Nebula Research") )
    misn.setReward( string.format(
        _("%s and the gratitude of a student"), creditstring(50000) ) )
    misn.setDesc( _("You have been asked by a Za'lek student to fly into the Nebula for some kind of research.") )
    misn_marker = misn.markerAdd(system.get(t_sys[1]), "low")
    
    -- Add mission
    misn.accept()
    local osd_title = _("Novice Nebula Research")
    local osd_msg = {}
    osd_msg[1] = string.format( _("Fly to the %s system"), t_sys[1] )
    osd_msg[2] = string.format( _("Fly to the %s system"), t_sys[2] )
    osd_msg[3] = string.format(
        _("Return to %s in the %s system"), homeworld:name(),
        homeworld_sys:name() )
    misn.osdCreate(osd_title, osd_msg)
    
    thook = hook.takeoff("takeoff")
    hook.land("land")
    hook.enter("jumpin")
end

function land()
    landed = planet.cur()
    if misn_stage == 3 and landed == homeworld then
        tk.msg( "", finish_text:format(
            homeworld:name(), creditstring(credits) ) )
        misn.cargoRm(cargo)
        player.pay(credits)
        misn.markerRm(misn_marker)
        zlk_addNebuResearchLog(
            _("You helped a Za'lek student to collect sensor data in the Nebula.") )
        misn.finish(true)
    end
end

function takeoff()
    local title = _("A Mess On Your Ship")
    tk.msg(title, string.format(takeoff_text, t_sys[1]))
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
    tk.msg("", string.format(scan_1_text, t_sys[1], t_sys[2]))
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
    tk.msg("", problems_text)
    hook.timer(10000, "stopProblems")
end

function stopProblems()
    ps:control(false)
    ps:setEnergy(100)
    tk.msg("", string.format(problems_end_text, t_sys[2]))
    misn_stage = 1
    misn.markerMove(misn_marker, system.get(t_sys[2]))
    misn.osdActive(2)
    hook.rm(phook)
    hook.enter("jumpin")
end

function beginSecondScan()
    tk.msg( "", string.format(
        _("You arrive in the %s system and Robert Hofer tells you that he will let you know when his scan is complete. This had better not cause another blackout..."),
        t_sys[2] ) )
    hook.timer(30000, "endSecondScan")
end

function endSecondScan()
    tk.msg( "", string.format(
        _([["OK, my measurements are complete! Let's go back to %s."]]),
        homeworld:name() ) )
    misn_stage = 3
    misn.markerMove(misn_marker, homeworld_sys)
    misn.osdActive(3)
end

