--[[
-- This is the first "prelude" mission leading to the FLF campaign.
--]]

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
   title = {}
   text = {}
   
   title[1] = "Gregar joins the party"
   text[1] = [[A haggard-looking man emerges from the airlock. He says, "Thank goodness you're here. My name is Gregar, I'm with the Frontier Liberation Front. I mean you no harm." He licks his lips in hesitation before continuing. "I have come under attack from a Dvaered patrol. I wasn't violating any laws, and we're not even in Dvaered territory! Anyway, my ship is unable to fly."
    You help Gregar to your cockpit and install him in a vacant seat. He is obviously very tired, but he forces himself to speak. "Listen, I was on my way back from a mission when those Dvaered bastards jumped me. I know this is a lot to ask, but I have little choice seeing how my ship is a lost cause. Can you take me the rest of the way? It's not far. We have a secret base in the %s system. Fly there and contact my comrades. They will take you the rest of the way."
   With that, Gregar nods off, leaving you to decide what to do next. Gregar wants you to find his friends, but harboring a known terrorist, let alone helping him, might not be looked kindly upon by the authorities...]]
   
   misn_title = "Save the FLF agent"
   misn_desc = "Take Gregar, the FLF agent to the %s system and make contact with the FLF"
end

function create()
    misn.accept()

    destsysname = var.peek("flfbase_sysname")
    destsys = system.get(destsysname)
    
    tk.msg(title[1], string.format(text[1], destsysname))
    
    misn.setTitle(misn_title)
    misn.setDesc(string.format(misn_desc, destsysname))
    
    hook.enter("enter")
end

function enter()
    if system.cur() == destsys then
        -- Add FLF base and waypoints
        dist = 1000 -- distance of the FLF base
        spread = 23 -- max degrees off-course for waypoints
        
        pilot.toggleSpawn(false)
        pilot.clear()
        faction.get("FLF"):modPlayerRaw(0) -- FLF is neutral to the player for this mission
        
        angle = var.peek("flfbase_angle")
        angle2 = angle + (rnd.rnd() - 0.5) * 2 * spread * 2 * math.pi / 360
        angle3 = angle + (rnd.rnd() - 0.5) * 2 * spread * 2 * math.pi / 360
        
        -- Base is far away in a random direction
        flfbajs = pilot.add("FLF Base", "flf", vec2.new(dist * math.cos(angle), dist * math.sin(angle)), false)
        -- Waypoints are 1/3 and 2/3 of the way away, at an angle plus or minus spread degrees from the actual base
        waypunt1 = pilot.add("Waypoint", "dummy", vec2.new(dist / 3 * math.cos(angle2), dist / 3 * math.sin(angle2)), false)
        waypunt2 = pilot.add("Waypoint", "dummy", vec2.new(2 * dist / 3 * math.cos(angle3), 2 * dist / 3 * math.sin(angle3)), false)

        waypoint1 = waypunt1[1]
        waypoint2 = waypunt2[1]
        flfbase = flfbajs[1]
        
        waypoint1:setInvincible(true)
        waypoint2:setInvincible(true)
        
        -- Add FLF ships that are to guide the player to the FLF base
        flfsheep = pilot.add("FLF Vendetta", "dummy", vec2.new(0,0), false)
        flfship = flfsheep[1]
        
        flfbase:rmOutfit("all")
        flfbase:addOutfit("Ripper MK2", 8)
        
    end
end

function abort()
    var.peek()
end