--[[
-- This is the first "prelude" mission leading to the FLF campaign. The player takes a FLF agent onboard, then either turns him in to the Dvaered or delivers him to a hidden FLF base.
-- stack variable flfbase_intro:
--      0 - The player has turned in the FLF agent.
--      1 - The player has rescued the FLF agent.
--      2 - The player has betrayed the FLF after rescuing the agent (not used in this script)
--]]

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    title = {}
    text = {}
    turnintitle = {}
    turnintext = {}
    misn_desc = {}
   
    title[1] = "Gregar joins the party"
    text[1] = [[A haggard-looking man emerges from the airlock. He says, "Thank goodness you're here. My name is Gregar, I'm with the Frontier Liberation Front. I mean you no harm." He licks his lips in hesitation before continuing. "I have come under attack from a Dvaered patrol. I wasn't violating any laws, and we're not even in Dvaered territory! Anyway, my ship is unable to fly."
    You help Gregar to your cockpit and install him in a vacant seat. He is obviously very tired, but he forces himself to speak. "Listen, I was on my way back from a mission when those Dvaered bastards jumped me. I know this is a lot to ask, but I have little choice seeing how my ship is a lost cause. Can you take me the rest of the way? It's not far. We have a secret base in the %s system. Fly there and contact my comrades. They will take you the rest of the way."
    With that, Gregar nods off, leaving you to decide what to do next. Gregar wants you to find his friends, but harboring a known terrorist, let alone helping him, might not be looked kindly upon by the authorities...]]
    
    title[2] = "Gregar puts an end to hostilities"
    text[2] = [["Wha- hey! What's going on!"
    You were too busy dodging incoming fire, rebalancing your shields and generally trying to kill your attackers before they kill you to notice that Gregar, your passenger, has roused from his slumber. Clearly the noise and the rocking have jolted him awake. You snap at him not to distract you from this fight, but he desperately interrupts.
    "These guys are my contacts, my friends! I was supposed to meet them here! Oh crap, this is not good. I didn't realize I'd be out this long! Look, I need to use your comm array right now. Trust me!"
    Before you have a chance to ask him what he thinks he's doing, Gregar begins tuning your communications array, and soon finds the frequency he wants.
    "FLF sentinel formation, this is Lt. Gregar Fletcher, authorization code six-six-niner-four-seven-Gamma-Foxtrot! Cease fire, I repeat, cease fire!" He then turns to you. "Same to you. Stop shooting. This is a misunderstanding, they're not your enemies."]]
    
    title[3] = "Gregar puts an end to hostilities"
    text[3] = [[You are skeptical at first, but a few seconds later it becomes apparent that the FLF fighters have indeed ceased firing. Then, there is an incoming comm from the lead ship.
    "This is FLF sentinel Alpha. Lt. Fletcher, authorization code verified. Why are you with that civilian? Where is your ship? And why didn't you contact us right away?"
    "Apologies, Alpha. It's a long story. For now, let me just tell you that you can trust the pilot of this ship. He has kindly helped me out of a desperate situation, and without him I probably would never have returned alive. Request you escort us to Sindbad."
    "Copy that Lt. Fletcher." Alpha then addresses you. "Please follow us. We will guide you to our base. Stay close. Sensor range is lousy in these parts, and if you get separated from us, we won't be able to find you again, and you won't be able to find us or our base."
    With that, Alpha breaks the connection. It seems you have little choice but to do as he says if you ever want to take Gregar to his destination.]]
    
    title[4] = "Gregar leaves the party"
    text[4] = [[]]
    
    commmsg = "Nothing personal, mate, but we're expecting someone and you ain't him. No witnesses!"
    
    contacttitle = "You have lost contact with your escorts!"
    contacttext = [[Your escorts have disappeared from your sensor grid. Unfortunately, it seems you have no way of telling where they went.
    
    You have failed to reach the FLF's hidden base.]]
    
    turnintitle[1] = "An opportunity to uphold the law"
    turnintext[1] = [[You have arrived at a Dvaered controlled world, and you are harboring a FLF fugitive on your ship. Fortunately, Gregar is still asleep. You could choose to alert the authorities and turn him in, and possibly collect a reward.
    Would you like to do so?]]
    turnintitle[2] = "Another criminal caught"
    turnintext[2] = [[It doesn't take Dvaered security long to arrive at your landing bay. They board your ship, seize Gregar and take him away before he even comprehends what's going on.
    "You have served House Dvaered adequately, citizen," the stone-faced captain of the security detail tells you. "In recognition of your service, we may allow you to participate in other operations regarding the FLF terrorists. If you have further questions, direct them to our public liaisons."
    The officer turns and leaves without even waiting for an answer, and without rewarding you in any tangible way. You wonder if you should scout out this liaison, in hopes of at least getting something out of this whole situation.]]
    
    misn_title = "Deal with the FLF agent"
    misn_desc[1] = "Take Gregar, the FLF agent to the %s system and make contact with the FLF"
    misn_desc[2] = "Alternatively, turn Gregar in to the nearest Dvaered base"
end

function create()
    misn.accept() -- The player chose to accept this mission by boarding the FLF ship

    flfdead = false -- Flag to check if the player destroyed the FLF sentinels
    
    destsysname = var.peek("flfbase_sysname")
    destsys = system.get(destsysname)
    
    tk.msg(title[1], string.format(text[1], destsysname))
    
    misn.osdCreate(misn_title, {string.format(misn_desc[1], destsysname), misn_desc[2]})
    
    misn.addCargo("Gregar", 1)
    
    hook.enter("enter")
    hook.land("land")
end

-- Handle the FLF encounter, Gregar's intervention, and ultimately the search for the base.
function enter()
    if system.cur() == destsys and not flfdead then
        dist = 15000 -- distance of the FLF base from the player jump-in point
        spread = 45 -- max degrees off-course for waypoints

        angle = var.peek("flfbase_angle")
        angle2 = angle + (rnd.rnd() - 0.5) * 2 * spread * 2 * math.pi / 360
        angle3 = angle + (rnd.rnd() - 0.5) * 2 * spread * 2 * math.pi / 360
        
        pilot.toggleSpawn(false)
        pilot.clear()

        faction.get("FLF"):modPlayerRaw(0) -- FLF is neutral to the player for this mission

        -- Pilot is to hyper in somewhere far away from the base.
        
        player.pilot():setPos(vec2.new(dist * math.cos(angle), dist * math.sin(angle)))

        -- Add FLF ships that are to guide the player to the FLF base (but only after a battle!)
        fleetFLF = pilot.add("FLF Vendetta Trio", "flf_nojump", player.pilot():pos(), false)
        faction.get("FLF"):modPlayerRaw(-200)
        
        player.pilot():setPos(player.pilot():pos() - player.pilot():vel() / 2.2) -- Compensate for hyperjump

        -- Add FLF base waypoints
        -- Base is at 0,0
        -- Waypoints are 1/3 and 2/3 of the way away, at an angle plus or minus spread degrees from the actual base
        waypunt0 = pilot.add("Waypoint", "dummy", vec2.new(0,0), false) -- The base will be spawned in the origin, but not until the player is close to this waypoint.
        waypunt1 = pilot.add("Waypoint", "dummy", vec2.new(dist / 3 * math.cos(angle2), dist / 3 * math.sin(angle2)), false)
        waypunt2 = pilot.add("Waypoint", "dummy", vec2.new(2 * dist / 3 * math.cos(angle3), 2 * dist / 3 * math.sin(angle3)), false)
        
        waypoint0 = waypunt0[1]
        waypoint1 = waypunt1[1]
        waypoint2 = waypunt2[1]
        
        waypoint1:setInvincible(true)
        waypoint2:setInvincible(true)
        
        misn.timerStart("commFLF", 2000)
        misn.timerStart("wakeUpGregarYouLazyBugger", 20000) -- 20s before Gregar wakes up
    end
end

-- There are two cases we need to check here: landing on the FLF base and landing on a Dvaered world.
function land()
    -- Case FLF base
    if planet.get():name() == "Sindbad" then
        tk.msg(title[4], text[4])
        var.push("flfbase_intro", 1)
        var.pop("flfbase_angle")
        misn.jetCargo("Gregar")
        misn.finish(true)
    end
    -- Case Dvaered planet
    elseif planet.get():faction():name() == "Dvaered" then
        if tk.yesno(turnintitle[1], turnintext[1]) then
            tk.msg(turnintitle[2], turnintext[2])
            var.push("flfbase_intro", 0)
            var.pop("flfbase_angle")
            misn.jetCargo("Gregar")
            misn.finish(true)
        end
    end
end

function commFLF()
    fleetFLF[1]:comm(commmsg)
end

-- Gregar wakes up and calls off the FLF attackers. They agree to guide you to their hidden base.
-- If the player has destroyed the FLF ships, nothing happens and a flag is set. In this case, the player can only do the Dvaered side of the mini-campaign.
function wakeUpGregarYouLazyBugger()
    for i, j in ipairs(fleetFLF) do
        j:setInvincible(true)
        j:setFriendly()
        j:changeAI("flf_nojump")
        flfship = j -- This is going to be the reference ship for conditionals.
    end
    if flfship:alive() then
        tk.msg(title[2], text[2])
        tk.msg(title[3], text[3])
        faction.get("FLF"):modPlayerRaw(100)
        misn.timerStart("toPoint2", 2000)
        misn.timerStart("outOfRange", 4000)
    else
        flfdead = true
    end
end

-- Part of the escort script
function toPoint2()
    for i, j in ipairs(fleetFLF) do
        j:changeAI(string.format("escort*%u", waypoint2:id()))
    end
    waytimer1 = misn.timerStart("toPoint1", 1000)
end

-- Part of the escort script
function toPoint1()
    if vec2.dist(flfship:pos(), waypoint2:pos()) < 300 then
        for i, j in ipairs(fleetFLF) do
            j:changeAI(string.format("escort*%u", waypoint1:id()))
        end
        waytimer0 = misn.timerStart("toPoint0", 1000)
    else
        waytimer1 = misn.timerStart("toPoint1", 1000)
    end
end

-- Part of the escort script
function toPoint0()
    if vec2.dist(flfship:pos(), waypoint1:pos()) < 300 then
        for i, j in ipairs(fleetFLF) do
            j:changeAI(string.format("escort*%u", waypoint0:id()))
        end
        basetimer = misn.timerStart("spawnbase", 1000)
    else
        waytimer0 = misn.timerStart("toPoint0", 1000)
    end
end

-- Part of the escort script
function spawnbase()
    if vec2.dist(flfship:pos(), waypoint0:pos()) < 1000 then
        diff.apply("FLF_base")
        misn.timerStop("outOfRange")
    else
        basetimer = misn.timerStart("spawnbase", 1000)
    end
end

-- Check if the player is still with his escorts
function outOfRange()
    if vec2.dist(flfship:pos(), player.pilot():pos()) < 1000 then
        misn.timerStart("outOfRange", 2000)
    else
        -- TODO: handle mission failure due to distance to escorts
        tk.msg(contacttitle, contacttext)
        for i, j in ipairs(fleetFLF) do
            j:setHealth(0, 0) -- Should really just remove them, not kill them.
        end
    end
end

function abort()
    var.pop("flfbase_angle")
    var.pop("flfbase_sysname")
    misn.jetCargo("Gregar")
    misn.finish(false)
end