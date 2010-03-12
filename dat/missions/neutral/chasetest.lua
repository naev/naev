--[[      Test for a chase mission
           In this mission, you will be chased by a pirate fleet across several systems.
           It is a testmission and has no text.
           
           MISSION: Chase Test
           DESCRIPTION: Pirates chase you to Gamma Polaris.
]]--

lang = naev.lang()
if lang == "es" then
else --I guess you know this stuff...
    NPC_name = "Anonymous" --NPC params
    bar_desc = "He is Legion"
    title = {}
    title[0] = "Chasetest" --Mission title
    title[1] = "Wild Goosechase" --dialogue title 1
    title[2] = "Good luck" --dialogue title 2
    title[3] = "Mission accomplished" --finished title
    text = {}
    text[0] = [[1. Take off
2. Survive until %s
3. Get support
4. Kill them
5. ????
6. Profit]] --dialogue 1
    text[1] = "You might need it" --dialogue 2
    text[2] = "Wow, you survived! But you're not paid. Anonymous doesn't pay." --finished
    misn_desc = "Flee from the Pirates to %s, then defeat them." --OSD text
    reward_desc = "A gazillion credits" --reward description
end

function create ()
    targetsystem = system.get("Gamma Polaris") --find target system
    
    misn.setNPC( NPC_name, "none") --spawn NPC
    misn.setDesc( bar_desc )
end

function accept ()
    if not tk.yesno( title[1], string.format( text[0], targetsystem:name() ) ) then --if accepted
        misn.finish()
    end
    
    misn.accept()
    tk.msg( title[2], text[1] ) --dialogue 2
    misn.setTitle( title[0] ) --OSD stuff
    misn.setReward( reward_desc )
    misn.setDesc( string.format( misn_desc, targetsystem:name() ) )
    misn.setMarker( targetsystem )
    
    hook.jumpin("jumpin") --trigger when entering a system
end

function jumpin () --aforementioned triggered function
    misn.timerStart("spawnBaddies",5000) --baddies spawn delayed
    if system.cur() == targetsystem then --when in Gamma Polaris
        defenders = pilot.add("Empire Sml Defense") --add a defending force to help you
        for pilot_number, pilot_object in pairs(defenders) do
            pilot_object:setFriendly() --I think they like you
        end
    end
end

function spawnBaddies () 
    pirates = {}
    pirates[0] = pilot.add("Pirate Admonisher")
    pirates[1] = pilot.add("Pirate Admonisher")
    pirates[2] = pilot.add("Pirate Admonisher")
    pirates_alive = 0
    for pirate_number, pirate_object in pairs(pirates) do
        for pilot_number, pilot_object in pairs(pirate_object) do
            pilot_object:setHostile() --they don't like you
            pilot_object:rmOutfit("all") --strip them down
            pilot_object:addOutfit("150mm Railgun") --add everything but rockets
            pilot_object:addOutfit("Plasma Blaster MK2")
            pilot_object:addOutfit("Ion Cannon")
            pilot_object:addOutfit("Reactor Class II")
            pilot_object:addOutfit("Milspec Jammer")
            pilot_object:addOutfit("Shield Capacitor II")
            pilot_object:addOutfit("Shield Capacitor III")
            pilot_object:addOutfit("Plasteel Plating")
            pilot_object:addOutfit("Engine Reroute")
            pilot_object:addOutfit("Battery II")
            pilot_object:control() --switch to manual control
            pilot_object:attack( pilot.player() ) --they blindly attack you
            pirates_alive = pirates_alive + 1
            hook.pilot(pilot_object, "death", "pilotKilled") --trigger when one of them is killed
        end
    end
end

function pilotKilled () --function for second trigger
    pirates_alive = pirates_alive - 1 --one less pirate alive
    if pirates_alive == 0 then --if none left
        tk.msg(title[3], text[2]) --finished
        misn.finish(true)
    end
end