--[[
-- This is a oneoff mission where you help a new Dvaered Warlord takeover a planet
-- To Do: fix fighters being idle after mission ends
-- Other editors, feel free to update dialog to make it more dvaered like.
--]]

-- localization stuff, translators would work here

include("fleethelper.lua")

lang = naev.lang()
if lang == "es" then
else -- default english
    destsysname = "Torg"
    destplanetname = "Jorcan"
    destjumpname = "Doranthex"

    title = {}
    text = {}
    failtitle = {}
    failtext = {}
    osd_desc = {}
    comm_msg = {}
    chatter = {}
    passtitle = {}
    passtext = {}

    title[1] = "The job offer"
    text[1] = [[    You walk up to the Dvaered official at his table. He mentions that he is looking for a pilot like yourself.
    "I am looking for a skilled pilot to do a simple job for me, interested?"]]
    
    title[2] = "A small distraction"
    text[2] = [[    "My General has just retired from the High Command and is now looking to become the Warlord of a planetary system. Unfortunately, our loyal forces do not seem sufficient enough to take on any existing planetary defense forces head on.  
    "However, it looks like there may be an opportunity for us in %s. Warlord Khan of %s has been building his newest flagship, the Hawk, and will be onboard the Hawk as it tests its hyperspace capabilities. Since its engines and weapons have not been fully installed yet, it will be substantially slower than normal and unable to defend itself.  
    "To protect himself and the Hawk, Khan will have deployed a substantial escort fighter fleet to defend against any surprise attack."]]
    text[3] = [[    "That is where you come in. You will jump into %s and find the Hawk and its escorts. Before the Hawk is able to reach hyperspace, you will fire on it, and cause the fighters to engage with you. At this point, you should run away from the Hawk and the jump point, so that the fighters will give chase. Then we will jump into the system and destroy the Hawk before the fighters can return."]]
    text[4] = [[    "We will jump in approximately 8000 STU after you jump into %s, so the fighters must be far enough away by then not to come back and attack us."]]
    
    refusetitle = "Nuts"
    refusetext = [[    "I see. In that case, I'm going to have to ask you to leave. My job is to recruit a civilian, but you're clearly not the man I'm looking for. You may excuse yourself, citizen."]]
    
    failtitle[1] = "You ran away!"
    failtext[1] = "You have left the system without first completing your mission. The operation ended in failure."

    failtitle[2] = "The Hawk got away!"
    failtext[2] = "The Hawk jumped out of the system. You have failed your mission."

    failtitle[3] = "The Hawk got away!"
    failtext[3] = "The Hawk landed back on %s. You have failed your mission."
    failtitle[4] = "The Hawk is safe."
    failtext[4] = "The Hawk was able to fend off the attackers and destroy their flagship. You have failed your mission."

    passtitle[1] = "The Dvaered official sent you a message."
    passtext[1] = [[   "Thanks for the distraction. I've sent you a picture of all the medals I was awarded. Oh, and I also deposited 80000 credits in your account."]]

    npc_desc = "A high ranking Dvaered officer. It looks like he might have a job offer for you."
    
    misn_title = "A Small Distraction"
    osd_desc[1] = "Fly to the %s system"
    osd_desc[2] = "Fire on the Hawk and flee from the fighter escorts until the Dvaered fleet jumps in and destroys the Hawk"  
    misn_desc = "You have been recruited to distract the Dvaered fighter escorts and lead them away from the jump gate and the capital ship Hawk. The Dvaered task force will jump in and attempt to destroy the Hawk before the escort ships can return. The mission will fail if the Hawk survives or the Dvaered task force is eliminated."

    chatter[0] = "All right men, this will be Hawk's maiden jump. Continue on course to the %s jump gate."
    chatter[1] = "How dare he attack me! Get him!"
    chatter[2] = "You heard Warlord Khan, blow him to pieces!"
    chatter[3] = "He's attacking us, blow him to pieces!"
    chatter[4] = "Arrgh!"
    chatter[5] = "Khan is dead! Who will be our warlord now?"
    chatter[6] = "Obviously the one who killed him, idiot!"
    chatter[7] = "I will never serve a different warlord than Khan! Die, you traitors!"
    chatter[8] = "%s will be ours! Khan, prepare to die!"
    chatter[9] = "All units, defend Hawk, we are under attack!"
    chatter[10] = "All units, defend Hawk, we are under attack!"
    chatter[11] = "Return to Hawk, Khan is in danger!"
    chatter[12] = "Pathetic, can't even take down an unarmed ship."
    chatter[13] = "I declare myself the Warlord of %s!"
end

function create()
    missys = {system.get(destsysname)}
    if not misn.claim(missys) then
        abort()
    end

    misn.setNPC("Dvaered liaison", "dvaered/dv_military_m1")
    misn.setDesc(npc_desc)
end

function accept()
    if tk.yesno(title[1], text[1]) then
        tk.msg(title[2], string.format(text[2], destsysname, destplanetname))
        tk.msg(title[2], string.format(text[3], destsysname))
        tk.msg(title[2], string.format(text[4], destsysname))

        misn.accept()
        osd_desc[1] = string.format(osd_desc[1], destsysname)
        misn.osdCreate(misn_title, osd_desc)
        misn.setDesc(misn_desc)
        misn.setTitle(misn_title)
        marker = misn.markerAdd( system.get(destsysname), "low" )
        
        missionstarted = false
        jump_fleet_entered = false
       
        hook.jumpout("jumpout")
        hook.enter("enter")
        hook.land("land")
    else
        tk.msg(refusetitle, refusetext)
        misn.finish()
    end
end

function jumpout()
    last_sys = system.cur()
end

function enter()
    if system.cur():name() == destsysname then
        pilot.toggleSpawn(false)
        pilot.clear()
        misn.osdActive(2)
        missionstarted = true
        j = jump.get(destsysname, destjumpname)
        v = j:pos()
        hawk = pilot.add("Dvaered Goddard", "dvaered_norun", v-vec2.new(1500,8000))[1]
        hawk:rename("Hawk")
        hawk:setHilight(true)
        hawk:setVisible(true)
        pilot.cargoAdd(hawk, "Food", 500)
        hawk:control()
        hawk:hyperspace(system.get(destjumpname))
        hawk:broadcast(string.format(chatter[0], destjumpname))
        fleetdv = pilot.add("Dvaered Home Guard", "dvaered_norun", hawk:pos()-vec2.new(1000,1500))
        for i, j in ipairs(fleetdv) do
            j:changeAI("dvaered_norun")
            j:setHilight(true)
            j:setVisible(true)
            j:control()
            j:goto(v)
            hook.pilot(j, "attacked", "fleetdv_attacked")
        end
        hook.pilot( hawk, "jump", "hawk_jump" )
        hook.pilot( hawk, "land", "hawk_land" )
        hook.pilot( hawk, "attacked", "hawk_attacked")
        hook.pilot( hawk, "death", "hawk_dead" )
        hook.timer(80000, "spawn_fleet")
    elseif missionstarted then -- The player has jumped away from the mission theater, which instantly ends the mission.
        tk.msg(failtitle[1], failtext[1])
        faction.get("Dvaered"):modPlayerSingle(-5)
        abort()
    end
end

function land()
    if missionstarted then -- The player has landed, which instantly ends the mission.
        tk.msg(failtitle[1], failtext[1])
        faction.get("Dvaered"):modPlayerSingle(-5)
        abort()
    end
end

function hawk_jump () -- Got away
    tk.msg(failtitle[2], failtext[2])
    faction.get("Dvaered"):modPlayerSingle(-5)
    hook.timer(10000, abort)
end

function hawk_land () -- Got away
    tk.msg(failtitle[3], failtext[3])
    faction.get("Dvaered"):modPlayerSingle(-5)
    hook.timer(10000, abort)
end

function hawk_attacked () -- chased
    if jump_fleet_entered then
    else
        hawk:broadcast(chatter[1])
        hawk:control()
        hawk:hyperspace(system.get(destjumpname))
        fleetdv[1]:broadcast(chatter[2])
    end
    for i, j in ipairs(fleetdv) do
        j:control()
        if jump_fleet_entered then 
           j:changeAI("dvaered_norun")
           j:control(false)
        else 
           j:attack(player.pilot())
        end
    end
end
function fleetdv_attacked () -- chased
    if jump_fleet_entered then
    else
        hawk:control()
        hawk:hyperspace(system.get(destjumpname))
        fleetdv[1]:broadcast(chatter[3])
    end
    for i, j in ipairs(fleetdv) do
    	if j:exists() then
           j:control()
           if jump_fleet_entered then 
               j:changeAI("dvaered_norun")
               j:control(false)
           else 
               j:attack(player.pilot())
           end
       end
    end
end

function hawk_dead () -- mission accomplished
    hawk:broadcast(chatter[4])
    fleetdv[1]:broadcast(chatter[5])
    fleetdv[2]:broadcast(chatter[6])
    fleetdv[3]:broadcast(chatter[7])
    fleetdv[1]:setFaction("FLF")
    fleetdv[2]:setFaction("FLF")
    fleetdv[4]:setFaction("FLF")
    fleetdv[5]:setFaction("FLF")
    fleetdv[7]:setFaction("FLF")
    fleetdv[8]:setFaction("FLF")
    fleetdv[10]:setFaction("FLF")
    fleetdv[11]:setFaction("FLF")
    fleetdv[13]:setFaction("FLF")
    fleetdv[14]:setFaction("FLF")
    for i, j in ipairs(fleetdv) do
        j:control(false)
        j:setVisible(false)
        j:setHilight(false)
    end
    hook.timer(10000, "complete")
    for i, j in ipairs(jump_fleet) do
        if j:exists() then
            j:land(planet.get(destplanetname))
	end    
    end
end

function spawn_fleet() -- spawn warlord killing fleet
    -- Cancel autonav.
    player.cinematics(true)
    player.cinematics(false)
    jump_fleet_entered = true
    jump_fleet = pilot.add("Dvaered Med Force", "dvaered_norun", system.get(destjumpname))
    jump_fleet[6]:broadcast(string.format(chatter[8], destplanetname))
    for i, j in ipairs(jump_fleet) do
        j:changeAI("dvaered_norun")
        j:setFaction("FLF")
        j:setHilight(true)
        j:setVisible()
        j:control()
        j:attack(hawk)
    end
    hook.pilot( jump_fleet[6], "death", "jump_fleet_cap_dead")
    camera.set(hawk)
    hawk:broadcast(chatter[9])
    fleetdv[1]:broadcast(chatter[10])
    hawk:control()
    hawk:land(planet.get(destplanetname))
    for i, j in ipairs(fleetdv) do
        j:changeAI("dvaered_norun")
        j:control(false)
        j:setFriendly()
    end
end
function jump_fleet_cap_dead () -- mission failed
    jump_fleet[6]:broadcast(chatter[4])
    hawk:broadcast(chatter[12])
    hawk:setNoDeath()
    tk.msg(failtitle[4], failtext[4])
    faction.get("Dvaered"):modPlayerSingle(-5)
    hawk:land(planet.get(destplanetname))
    for i, j in ipairs(fleetdv) do
        j:control()
        j:follow(hawk)
        j:setHilight(false)
    end
    for i, j in ipairs(jump_fleet) do
        j:control()
        j:follow(hawk)
        j:setHilight(false)
    end
    hook.timer(10000, abort)
end      

function complete()
    tk.msg(passtitle[1], passtext[1])
    camera.set(player.pilot())
    player.pay(80000)
    jump_fleet[6]:broadcast(string.format(chatter[13], destplanetname))
    misn.finish(true)
end

function abort()
    camera.set(player.pilot(), true)
    misn.finish(false)
end
