--[[      Test for a chase mission
           In this mission, you will be chased by a pirate fleet across several systems.
           It is a testmission and has no text.
           
           MISSION: Chase Test
           DESCRIPTION: Pirates chase you to Gamma Polaris.
]]--

lang = naev.lang()
if lang == "es" then
elseif lang == "de" then
else --I guess you know this stuff...
    NPC_name = "A detective" --NPC params
    bar_desc = "A private detective is signalling you to come speak with him"
    title = {}
    title[0] = "Crimelord" --Mission title
    title[1] = "Good luck" --dialogue title
    title[2] = "Mission accomplished" --finished title
    title[3] = "He told you so..." --failed title
    text = {}
    text[0] = [[    The private detective greets you and gets right down to business.
    "I have tracked down and collected evidence against a local crimelord", he says. "The Evidence is on this datadisk. He would love nothing more than to get his hands on this.
    I want you to bring this to my associates in the %s system, they'll see to it that this man is put into Jail. Though I should warn you: There is a reason I'm giving this job to you and no one else:
    They will come after you and you need a fast ship to shake them off. You will be paid when you arrive in the %s system. My associates will compensate you generously.
    Oh, before I forget: There is another catch: You can NOT land until you get to the system. They will get you and take the evidence, so I repeat, do NOT land!"
    You are unsure whether that sounds appealing to you, do you accept?]] --dialogue 1
    text[1] = "You might need it" --dialogue 2
    text[2] = "\"Thank you, that will surely end this guys reign of crimes. Now about your compensation. I can give you %s credits.\"" --finished
    text[3] = "No sooner than you step out of your ship do the crimelords thugs sorround you. They are heavily armed and you see no other way to get away with your life than to give them the evidence."
    misn_desc = "Flee from the thugs to %s, then defeat them." --OSD text
    reward_desc = "A generous compensation" --reward description
end

function create ()
    targetsystem = system.get("Ogat") --find target system
    
    misn.setNPC( NPC_name, "thief2") --spawn NPC
    misn.setDesc( bar_desc )
end

function accept ()
    if not tk.yesno( title[0], string.format( text[0], targetsystem:name(), targetsystem:name() ) ) then --if accepted
        misn.finish()
    end
    
    misn.accept()
    reward = 40000
    tk.msg( title[1], text[1] ) --dialogue 2
    misn.setTitle( title[0] ) --OSD stuff
    misn.setReward( reward_desc )
    misn.setDesc( string.format( misn_desc, targetsystem:name() ) )
    misn.setMarker( targetsystem )
    
    startsystem = system.cur() --needed to make thugs appear random in the first system
    last_system = system.cur() --ignore this one, it's just the intitiation of the variable
    
    hook.jumpin("jumpin") --trigger when entering a system
    hook.jumpout("jumpout") --trigger when leaving a system
    hook.land("land") --trigger when landing
    
end

function jumpin () --aforementioned triggered function
    misn.timerStart("spawnBaddies",4000) --baddies spawn delayed
    
    if system.cur() == targetsystem then --when in target system
    
        defenders = pilot.add("Mission Associate") --add a defending force to help you
        for pilot_number, pilot_object in pairs(defenders) do
            pilot_object:setFriendly() --I think they like you
        end
        
        capship = pilot.add("Mission Kestrel") --add the capship - needed for the mission
        for cap_num, cap_obj in pairs(capship) do
            cap_obj:setInvincible(true) --since it's needed it may not be destroyed
            cap_obj:setFriendly()
        end
    end
end

function jumpout ()
    last_system = system.cur() --save system you came from
end

function spawnBaddies ()
    if last_system == startsystem then
        thugs = pilot.add( "Mission Thugs")
    else
        thugs = pilot.add( "Mission Thugs", nil, last_system)
    end
    thugs_alive = 0
    for pilot_number, pilot_object in pairs(thugs) do
        pilot_object:setHostile() --they don't like you
        pilot_object:rmOutfit("all") --strip them down
        pilot_object:addOutfit("Laser Cannon MK2") --add everything but rockets
        pilot_object:addOutfit("Plasma Blaster MK2")
        pilot_object:addOutfit("Plasma Blaster MK2")
        pilot_object:addOutfit("Reactor Class II")
        pilot_object:addOutfit("Milspec Jammer")
        pilot_object:addOutfit("Shield Capacitor II")
        pilot_object:addOutfit("Shield Capacitor III")
        pilot_object:addOutfit("Plasteel Plating")
        pilot_object:addOutfit("Engine Reroute")
        pilot_object:addOutfit("Battery II")
        pilot_object:addOutfit("Auxiliary Processing Unit II")
        if system.cur() ~= targetsystem then
            pilot_object:control() --switch to manual control
            pilot_object:attack( pilot.player() ) --they blindly attack you and only you
        else
            thugs_alive = thugs_alive + 1
            hook.pilot(pilot_object, "death", "pilotKilled") --trigger when one of them is killed
        end
    end
    thugs[1]:comm("We'll get you!",true)
end

function pilotKilled () --function for second trigger
    thugs_alive = thugs_alive - 1 --one less thug alive
    if thugs_alive == 0 then --if none left
        capship[1]:hailPlayer()
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