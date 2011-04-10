--[[      Test for a chase mission
           In this mission, you will be chased by a pirate fleet across several systems.
           It is a testmission and has no text.
           
           MISSION: Chase Test
           DESCRIPTION: Pirates chase you to Gamma Polaris.
]]--

include "scripts/fleethelper.lua"

lang = naev.lang()
if lang == "es" then
else --I guess you know this stuff...
    NPC_name = "A detective" --NPC params
    bar_desc = "A private detective is signalling you to come speak with him."
    title = {}
    title[0] = "Crimelord" --Mission title
    title[1] = "Good luck" --dialogue title
    title[2] = "Mission Accomplished" --finished title
    title[3] = "He told you so..." --failed title
    text = {}
    text[0] = [[    The private detective greets you and gets right down to business.

    "I have tracked down and collected evidence against a local crimelord," he says. "The evidence is on this data disk. He would love nothing more than to get his hands on this.

    I want you to bring this to my associates in the %s system. While the local authorities have proven corruptible, my associates will ensure that this man ends up in prison, where he belongs. I must warn you, however:

    He is a man of not-inconsiderable influence. He has many friends, and no doubt will send some of his mercenaries to stop you. You'll need a fast ship to shake them off. My associates will compensate you generously when you reach %s.

    Regrettably, you are not the first pilot I've contacted regarding this matter. Your predecessor was intercepted when he landed en route to %s. The crime lord has many underlings lurking in nearby spaceports, you must NOT land until you've delivered the data."

    Given the dangers, you're not sure whether the reward will make this worth your while. Do you accept?]] --dialogue 1
    text[1] = [[    After quickly glancing around to make sure nobody's taken a particular interest, the detective presses the data stick into your hand.

    "Be careful out there. I doubt you'll be able to get far without being noticed."]] --dialogue 2
    text[2] = "\"Excellent work. This data will ensure an arrest and swift prosecution. You've certainly done your part towards cleaning up the region As for your compensation, I've had %s credits transferred to you.\"" --finished
    text[3] = "As you step out of your ship and seal the airlock, you spot a burly man purposefully heading towards you. You turn to flee, but there are others closing in on your position. Surrounded, and with several laser pistols trained on you, you see no option but to surrender the evidence."
    misn_desc = "Evade the thugs and deliver the evidence to %s." --OSD text
    reward_desc = "A generous compensation" --reward description
end

function create ()
    targetsystem = system.get("Ogat") --find target system
    
    misn.setNPC( NPC_name, "thief2") --spawn NPC
    misn.setDesc( bar_desc )
end

function accept ()
    -- Note: this mission does not make any system claims.
    if not tk.yesno( title[0], string.format( text[0], targetsystem:name(),
         targetsystem:name(), targetsystem:name() ) ) then --if accepted
        misn.finish()
    end
    
    misn.accept()
    reward = 40000
    tk.msg( title[1], text[1] ) --dialogue 2
    misn.setTitle( title[0] ) --OSD stuff
    misn.setReward( reward_desc )
    misn.setDesc( string.format( misn_desc, targetsystem:name() ) )
    misn.markerAdd( targetsystem, "low" )
    misn.osdCreate(title[0], {misn_desc:format(targetsystem:name())})

    startsystem = system.cur() --needed to make thugs appear random in the first system
    last_system = system.cur() --ignore this one, it's just the intitiation of the variable
    
    hook.enter("enter") --trigger when entering a system
    hook.jumpout("jumpout") --trigger when leaving a system
    hook.land("land") --trigger when landing
    
end

function enter () --aforementioned triggered function
    hook.timer(4000, "spawnBaddies")
    
    if system.cur() == targetsystem then --when in target system
        local defenderships = { "Lancelot", "Lancelot", "Admonisher", "Pacifier",
                "Hawking", "Kestrel" }
        defenders = addRawShips( defenderships, "dvaered", system.jumpPos(targetsystem,
                last_system), "Associates" ) --add a defending force to help you
        renameShips( defenders, "^", "Associate " )
        for pilot_number, pilot_object in pairs(defenders) do
            pilot_object:setFriendly() --I think they like you
            pilot_object:setPos( pilot_object:pos() +
                    vec2.new( rnd.rnd(400, 800) * (rnd.rnd(0,1) - 0.5) * 2,
                    rnd.rnd(400, 800) * (rnd.rnd(0,1) - 0.5) * 2))
        end

        capship = defenders[#defenders]
        capship:setInvincible()
        capship:comm("We've got your back. Engaging hostiles.", true )
    end
end

function jumpout ()
    last_system = system.cur() --save system you came from
end

function spawnBaddies ()
    if last_system == startsystem then
        ai = "baddie"
    else
        ai = "baddie_norun"
    end
    thugs = addRawShips( "Admonisher", ai, last_system, "Thugs", 4 )
    renameShips( thugs, "^.*", "Thug" )
    for pilot_number, pilot_object in ipairs(thugs) do
        pilot_object:setHostile() --they don't like you
        pilot_object:rmOutfit("all") --strip them down
        pilot_object:addOutfit("Laser Cannon MK2") --add everything but rockets
        pilot_object:addOutfit("Plasma Blaster MK2", 2)
        pilot_object:addOutfit("Vulcan Gun", 2)
        pilot_object:addOutfit("Reactor Class II", 2)
        pilot_object:addOutfit("Milspec Jammer")
        pilot_object:addOutfit("Engine Reroute")
        pilot_object:addOutfit("Steering Thrusters")
        pilot_object:addOutfit("Shield Capacitor II")
        if system.cur() ~= targetsystem then
            pilot_object:control() --switch to manual control
            pilot_object:attack( pilot.player() ) --they blindly attack you and only you
        else
            thugs_alive = #thugs
            hook.pilot(pilot_object, "exploded", "pilotKilled") --trigger when one of them is killed
        end
    end
    threats = {
      "Surrender now and we'll let you live.",
      "You're dead!",
      "You won't make it out alive!",
      "Get back here!"
    }
    thugs[1]:comm(threats[rnd.rnd(1,#threats)],true)
end

function pilotKilled () --function for second trigger
    thugs_alive = thugs_alive - 1 --one less thug alive
    if thugs_alive == 0 then --if none left
        capship:hailPlayer()
        hook.pilot(capship[1], "hail", "capHailed")
    end
end

function capHailed () --when hailing the capship back
    tk.msg( title[2], string.format( text[2], reward) ) --congratulates
    player.pay( reward )
    misn.finish(true)
end

function land () --when landing
    tk.msg( title[3], text[3]) --you fail
    misn.finish(false)
end
