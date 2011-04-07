--[[
-- This is the second mission in the anti-FLF Dvaered campaign. The player is part of a Dvaered plot to smoke out the FLF base.
-- stack variable flfbase_intro:
--      1 - The player has turned in the FLF agent or rescued the Dvaered crew. Conditional for dv_antiflf02
--      2 - The player has rescued the FLF agent. Conditional for flf_pre02
--      3 - The player has found the FLF base for the Dvaered, or has betrayed the FLF after rescuing the agent. Conditional for dv_antiflf03
--]]

-- localization stuff, translators would work here

include("scripts/fleethelper.lua")

lang = naev.lang()
if lang == "es" then
else -- default english
    title = {}
    text = {}
    failtitle = {}
    failtext = {}
    osd_desc = {}
    
    title[1] = "House Dvaered needs YOU"
    text[1] = [[    You join the Dvaered official at his table. He greets you in a cordial fashion, at least by Dvaered standards. You explain to him that you wish to know more about the anti-FLF plans that House Dvaered is supposedly working on.
    "Ah! It's good to see such righteous enthusiasm amoung our citizenry. Indeed, our forces have been preparing to deal a significant blow to the terrorists of the FLF. I can't tell you the details, of course - this is a military operation. However, if you have a combat-capable ship and enough sense of duty to assist us, there is an opportunity to serve alongside the real Dvaered warriors. How about it? Can we count on our support?"]]
    
    title[2] = "A clever ruse"
    text[2] = [[    "Splendid. You have the right mindset, citizen. If only all were like you! But that is neither here nor there. Come, I will take you to the local command bunker. The details of this operation will be explained to you there."
    
    True to his word, the Dvaered liaison escorts you out of the spaceport bar, and within the hour you find yourself deep inside a highly secured Dvaered military complex. You are ushered into a room, in which you find a large table and several important-looking military men. At the head of the table sits a man whose name tag identifies him as a Colonel Urnus. Evidently he's the man in charge.
    You are told to take a seat.]]
    
    title[3] = ""
    text[3] = [[    Colonel Urnus starts the meeting. "Welcome, citizen. You know why you are here and you know what this meeting is about, so let me get right to the point. We have reason to believe the FLF terrorists are operating from a secret base of operations. We know this base is located somewhere in the nebula, and we have recently uncovered intel that indicates the base may be in the %s system."
    One of the walls lights up, showing part of the galaxy map. The %s system is colored red, and it's pulsating gently.
    "Of course, we have conducted patrols in this system, but so far without result. Our sensors are badly impaired by the nebula in this system, so the chances of us finding the terrorist hive by our own devices are slim. Fortunately, our top strategists have come up with a ruse, one that will make the FLF come to us.
    "We will use a civilian ship - your ship, naturally - as a decoy," Urnus continues. The image on the wall zooms up to the %s system, and a white blip representing your ship appears near the jumpout point. "Your ship will be equipped with an IFF transponder in use by the FLF, so to anyone who isn't looking too closely you will appear as an FLF ship. Of course, this alone is not enough. The FLF will assume you know where there base is, since you look like one of them."
    The image on the wall updates again, this time showing several House Dvaered logos close to your ship.
    "Some time after you enter the system, several of our military assets will jump in and open fire. To the FLF, it will look like one of their own has come under attack! Since their base is nearby, they will undoubtably send reinforcements to help their 'comrade' out of a tight situation."]]
    
    title[4] = ""
    text[4] = [[    "As soon as the FLF ships join the battle, you and the Dvaered ships will disengage and target the FLF instead. Your mission is to render at least one of their ships incapable of fighting, and board it. You can then access the ship's computer and download the flight log, which will include the location of the FLF base. Take this information to a Dvaered base, and your mission will be complete."
    The image on the wall updates one last time, simulating the battle as described by Colonel Urnus. Several FLF logos appear, which are promptly surrounded by the Dvaered ones. Then the logos turn gray, indicating that they've been disabled.
    "Let me make one thing clear, citizen. You are allowed, even expected to fire on the Dvaered ships that are firing on you. However, you must make it look you're on the losing side, or the FLF will not come to your aid! So, do NOT disable or destroy any Dvaered ships, and make sure your own armor takes a bit of a beating. This is vital to the success of the mission. Do not fail."
    Colonel Urnus to have concluded his explanation, so you, having spotted the obvious flaw in the Dvaereds' plan, pop the question of what happens if the FLF never show up.
    "Well," the Colonel muses, "That will mean our intel was probably wrong. But don't worry, citizen, we'll get those terrorists eventually! Now, time is of the essence, so get to your ship and follow your orders. Dismissed!"]]
    
    title[5] = "Take no prisoners - only their logs"
    text[5] = [[You successfully board the FLF ship and secure its flight logs. This is what the Dvaered want - you should take it to a Dvaered planet immediately.]]
    
    title[6] = "X marks the spot"
    text[6] = [[    As soon as you land, a Dvaered military operator contacts you and requests you turn over the flight log you procured from the FLF ship, so you do. The Dvaered are then silent for some twenty minutes, time you use to complete your post-landing routines. Then, you are summoned to the local Dvaered security station.
    Colonel Urnus welcomes you. "Well met, citizen. I have received word of your accomplishment in our recent operation. It seems HQ is quite pleased with the result, and they have instructed me to reward you appropriately."
    He hands you a credit chip that represents a decent sum of money, though you feel that a mere monetary reward doesn't begin to compensate for the insane plan the Dvaered made you part of. However, you wisely opt not to give voice to that thought.
    "In addition," Urnus resumes, "Dvaered military command has decided that you may participate in the upcoming battle against the FLF stronghold, in recognition of your courage and your skill in battle. You may contact our liaison whenever you're ready."
    That concludes the pleasantries, and you are unceremoniously deposited outside the security compound. But at least you earned some money - and a chance to see some real action.]]
    
    refusetitle = "House Dvaered is out of luck"
    refusetext = [[    "I see. In that case, I'm going to have to ask you to leave. My job is to recruit a civilian, but you're clearly not the man I'm looking for. You may excuse yourself, citizen."]]
    
    failtitle[1] = "You ran away!"
    failtext[1] = "You have left the system without first completing your mission. The operation ended in failure."
    
    failtitle[2] = "You fought too hard!"
    failtext[2] = "You have disabled a Dvaered ship, thereby violating your orders. The operation is canceled thanks to you. The Dvaered are less than pleased."
    
    failtitle[3] = "All the FLF are dead!"
    failtext[3] = "Unfortunately, all the FLF ships have been destroyed before you managed to board one of them. The operation ended in failure."
    
    npc_desc = "This must be the Dvaered liaison you heard about. Allegedly, he may have a job to you that involves fighting the Frontier Liberation Front."
    
    misn_title = "Lure out the FLF"
    osd_desc[1] = "Fly to the %s system"
    osd_desc[2] = "Wait for the Dvaered military to jump in and attack you"
    osd_desc[3] = "Fight the Dvaered until the FLF step in"
    osd_desc[4] = "Disable and board at least one FLF ship"
    osd_desc[5] = "Return to any Dvaered world"
    
    misn_desc = "You have been recruited to act as a red herring in a military operation of Dvaered design. Your chief purpose is to goad the FLF into showing themselves, then disabling and boarding one of their ships. You will fail this mission if you disable or destroy any Dvaered ship, or if you leave the system before the operation is complete."
    
    comm_msg = "Here come the FLF! All units, disable the terrorist ships!"
end

function create()
    missys = {system.get(var.peek("flfbase_sysname"))}
    if not misn.claim(missys) then
        abort()
    end

    misn.setNPC("Dvaered liaison", "dv_liason")
    misn.setDesc(npc_desc)
end

function accept()
    if tk.yesno(title[1], text[1]) then
        destsysname = var.peek("flfbase_sysname")
        tk.msg(title[2], text[2])
        tk.msg(title[2], string.format(text[3], destsysname, destsysname, destsysname))
        tk.msg(title[2], text[4])

        misn.accept()
        osd_desc[1] = string.format(osd_desc[1], destsysname)
        misn.osdCreate(misn_title, osd_desc)
        misn.setDesc(misn_desc)
        misn.setTitle(misn_title)
        misn.markerAdd( system.get(destsysname), "low" )
        
        missionstarted = false
        DVdisablefail = true
        logsfound = false
        flfdead = 0
        
        misn.cargoAdd("FLF IFF Transponder", 0)
       
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
    if system.cur():name() == destsysname and not logsfound then
        pilot.clear()
        pilot.toggleSpawn(false)
        misn.osdActive(2)
        hook.timer(15000, "spawnDV")
    elseif missionstarted then -- The player has jumped away from the mission theater, which instantly ends the mission.
        tk.msg(failtitle[1], failtext[1])
        faction.get("Dvaered"):modPlayerRaw(-10)
        abort()
    end
end

function land()
    if logsfound and planet.cur():faction():name() == "Dvaered" then
        tk.msg(title[6], text[6])
        var.push("flfbase_intro", 3)
        faction.get("Dvaered"):modPlayerRaw(5)
        player.pay(70000) -- 70K
        misn.finish(true)
    end
end

function spawnDV()
    misn.osdActive(3)
    missionstarted = true
    fleetDV = pilot.add("Dvaered Strike Force", "norun", last_sys)
    -- The Dvaered ships should attack the player, so set them hostile.
    -- These are Vigilances, so we should tune them WAY down so the player doesn't insta-die.
    for i, j in ipairs(fleetDV) do
        j:setHostile()
        j:setHilight(true)
        j:setVisplayer(true)
        j:rmOutfit("all")
        j:addOutfit("Plasma Blaster MK2", 2)
        j:addOutfit("Shield Booster", 1)
        j:addOutfit("Steering Thrusters", 1)
        j:addOutfit("Solar Panel", 1)
        hook.pilot(j, "disable", "disableDV")
    end
    
    hook.timer(500, "pollHealth")
end

-- Polls the player's health and the Dvaereds' shields, and spawns the FLF fleet if shields and armor are below a certain value.
function pollHealth()
    shieldDV = 0
    parmor, pshield = player.pilot():health()
    for i, j in ipairs(fleetDV) do
        armor, shield = j:health()
        shieldDV = shieldDV + shield
    end
    if parmor <= 70 and pshield <= 10 and shieldDV <= 250 then
        spawnFLF()
    else
        hook.timer(500, "pollHealth")
    end
end

-- Spawn the FLF ships, reset the Dvaered.
function spawnFLF()
    DVdisablefail = false -- The player no longer fails the mission if a Dvaered ship is disabled
    misn.osdActive(4)
    for i, j in ipairs(fleetDV) do
        j:setFriendly()
        j:setHilight(false)
        j:changeAI("norun")
    end
    angle = rnd.rnd() * 2 * math.pi
    dist = 800
    vecFLF = vec2.new(math.cos(angle) * dist, math.sin(angle) * dist)
    fleetFlf = addShips( "FLF Vendetta", "flf_norun", player.pilot():pos() + vecFLF, 6 )
    fleetDV[1]:comm(comm_msg)
    
    for i, j in ipairs(fleetFLF) do
        j:setHilight(true)
        j:setVisplayer()
        hook.pilot(j, "disable", "disableFLF")
        hook.pilot(j, "death", "deathFLF")
        hook.pilot(j, "board", "boardFLF")
    end
end

function disableDV()
    if DVdisablefail then -- Only true as long as the FLF aren't there yet
        tk.msg(failtitle[2], failtext[2])
        faction.get("Dvaered"):modPlayerRaw(-10)
        abort()
    end
end

function disableFLF()
    -- Persuade the Dvaered to stop shooting at disabled FLF
    for i, j in ipairs(fleetDV) do
        if j:exists() then
            j:changeAI("norun")
        end
    end
end

function deathFLF()
    flfdead = flfdead + 1
    if flfdead == 6 and not logsfound then
        tk.msg(failtitle[3], failtext[3])
        abort()
    end
end

function boardFLF()
    misn.osdActive(5)
    tk.msg(title[5], text[5])
    missionstarted = false
    logsfound = true
    player.unboard()
    for i, j in ipairs(fleetDV) do
        if j:exists() then
            j:changeAI("flee") -- Make them jump out (if they're not dead!)
        end
    end
    for _, j in ipairs(fleetFLF) do
        if j:exists() then
            j:setHilight(false)
            j:setVisplayer(false)
            j:setNoboard(true)
        end
    end
end

function abort()
    misn.finish(false)
end
