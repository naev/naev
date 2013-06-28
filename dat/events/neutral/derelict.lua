--[[
-- Derelict Event
--
-- Creates a derelict ship that spawns random events.
--]]

lang = naev.lang()
if lang == "es" then
    -- not translated atm
else -- default english 
    -- Text goes here. Please keep the section as organized as possible.

    --=== NEUTRAL EVENTS ===--
    ntitle = "Empty derelict"
    ntext = {}
    ntext[1] = [[You spend some time searching the derelict, but it doesn't appear there are any remaining passengers, nor is there anything of interest to you. You decide to return to your ship.]]
    ntext[2] = [[This ship has clearly been abandoned a long time ago. Looters have been here many times, and all of the primary and backup systems are down. However, there is one console that is still operational. It appears to be running an ancient computer game about space exploration, trade and combat. Intrigued, you decide to give the game a try, and before long you find yourself hooked on it. You spend many fun hours hauling cargo, upgrading your ship, fighting enemies and exploring the universe. But of course you can't stay here forever. You regretfully leave the game behind and return to the reality of your everyday life.]]
    ntext[3] = [[You are exploring the derelict ship when you hear a strange creaking noise. You decide to follow it to see what is causing it, but you never seem to be able to pinpoint the source. After about an hour of fruitlessly opening up panels, pressing your ear against the deck and running hull scans from your own ship, you decide to give up and leave the derelict to its creepy creaking.]]
    ntext[4] = [[While exploring the cockpit of this derelict, you come across the captain's logs. Hoping to find some information that could be of use to you, you decide to play back the most recent entries. Unfortunately, it turns out the captain's logs are little more than recordings of the captain having heated arguments with her co-pilot. After about ten minutes, you decide you probably aren't going to find anything worthwhile here, so you return to your ship.]]
    ntext[5] = [[This derelict is not deserted. The crew are still onboard. Unfortunately for them, they didn't survive whatever wrecked their ship. You decide to give them a decent space burial before moving on.]]
    ntext[6] = [[This derelict seems to have been visited by looters already. You find a message carved into the wall near the airlock. It reads: "I WUS HEAR". Below it is another carved message that says "NO U WASNT". Otherwise, there is nothing of interest left on this ship.]]
    ntext[7] = [[This derelict seems to have at one time been used as an illegal casino. There are roulette tables and slot machines set up in the cargo hold. However, it seems the local authorities caught wind of the operation, because there are also scorch marks on the furniture and the walls, and there are assault rifle shells underfoot. You don't reckon you're going to find anything here, so you leave.]]
    ntext[8] = [[When the airlock opens, you are hammered in the face by an ungodly smell that almost makes you pass out on the spot. You hurriedly close the airlock again and flee back into your own ship. Whatever is on that derelict, you don't want to find out!]]
    ntext[9] = [[This derelict has really been beaten up badly. Most of the corridors are blocked by mangled metal, and your scans read depressurized compartments all over the ship. There's not much you can do here, so you decide to leave the derelict alone.]]
    ntext[10] = [[The interior of this ship is decorated in a gaudy fashion. There are cute plushies hanging from the doorways, drapes on every viewport, colored pillows in the corners of most compartments and cheerful graffiti on almost all the walls. A scan of the ship's computer shows that this ship belonged to a trio of adventurous young ladies who decided to have a wonderful trip through space. Sadly, it turned out none of them really knew how to fly a space ship, and so they ended up stranded and had to be rescued. Shaking your head, you return to your own ship.]]
    ntext[11] = [[The artificial gravity on this ship has bizarrely failed, managing to somehow reverse itself. As soon as you step aboard you fall upwards and onto the ceiling, getting some nasty bruises in the process. Annoyed, you search the ship, but without result. You return to your ship - but forget about the polarized gravity at the airlock, so you again smack against the deck plates.]]
    ntext[12] = [[The cargo hold of this ship contains several heavy, metal chests. You pry them open, but they are empty. Whatever was in them must have been pilfered by other looters already. You decide not to waste any time on this ship, and return to your own.]]
    ntext[13] = [[You have attached your docking clamp to the derelict's airlock, but the door refuses to open. A few diagnostics reveal that the other side isn't pressurized. The derelict must have suffered hull breaches over the years. It doesn't seem like there's much you can do here.]]
    ntext[14] = [[As you walk through the corridors of the derelict, you can't help but notice the large scratch marks on the walls, the floor and even the ceiling. It's as if something went on a rampage throughout this ship - something big, with a lot of very sharp claws and teeth... You feel it might be best to leave as soon as possible, so you abandon the search of the derelict and disengage your docking clamp.]]
    
    --=== GOOD EVENTS ===--
    gtitle = "Lucky find!"
    gtext = {}
    gtext[1] = [[The derelict appears deserted, its passengers long gone. However, they seem to have left behind a small amount of credit chips in their hurry to leave! You decide to help yourself to them, and leave the derelict.]]
    gtext[2] = [[The derelict is empty, and seems to have been thoroughly picked over by other space bucceneers. However, the ship's computer contains a map of the %s! You download it into your own computer.]]
    gtext[3] = [[This ship looks like any old piece of scrap at a glance, but it is actually an antique, one of the very first of its kind ever produced! Museums all over the galaxy would love to have a ship like this. You plant a beacon on the derelict to mark it for salvaging, and contact the %s authorities. Your reputation with them has slightly improved.]]
    
    --=== BAD EVENTS ===--
    btitle = "Oh no!"
    btext = {}
    btext[1] = [[The moment you affix your boarding clamp to the derelect ship, it triggers a boobytrap! The derelict explodes, severely damaging your ship. You escaped death this time, but it was a close call!]]
    btext[2] = [[You board the derelict ship and search its interior, but you find nothing. When you return to your ship, however, it turns out there were Space Leeches onboard the derelict - and they've now attached themselves to your ship! You scorch them off with a plasma torch, but it's too late. The little buggers have already drunk all of your fuel. You're not jumping anywhere until you find some more!]]
    btext[3] = [[You affix your boarding clamp and walk aboard the derelict ship. You've only spent a couple of minutes searching the interior when there is a proximity alarm from your ship! Pirates are closing on your position! Clearly this derelict was a trap! You run back onto your ship and prepare to unboard, but you've lost precious time. The pirates are already in firing range...]]

    --=== MISSION EVENTS ===--
    -- Add the name of your mission to this list. It will automatically have a chance of triggering from the derelict. The event handles unboarding.
    missionlist = {}
--    missionlist[1] = ""
end 


function create ()

    -- Get the derelict's ship.
    r = rnd.rnd()
    if r > 0.8 then
        ship = "Trader Gawain"
    elseif r > 0.6 then
        ship = "Trader Mule"
    elseif r > 0.4 then
        ship = "Trader Koala"
    else 
        ship = "Trader Llama"
    end
    
    -- Create the derelict.
    angle = rnd.rnd() * 2 * math.pi
    dist  = rnd.rnd(400, system.cur():radius() * 0.6)
    pos   = vec2.new( dist * math.cos(angle), dist * math.sin(angle) )
    p     = pilot.add(ship, "dummy", pos)[1]
    p:setFaction("Derelict")
    p:disable()
    p:rename("Derelict")
    hook.pilot(p, "board", "board")
    hook.pilot(p, "death", "destroyevent")
    hook.jumpout("destroyevent")
    hook.land("destroyevent")
end

function board()
    player.unboard()
    -- Roll for events
    local prob = rnd.rnd()
    if prob <= 0.50 then
        neutralevent()
    elseif prob <= 0.70 then
        goodevent()
    elseif prob <= 0.85 then
        badevent()
    else
        missionevent()
    end
end

function neutralevent()
    -- Pick a random message from the list, display it, unboard.
    tk.msg(ntitle, ntext[rnd.rnd(1, #ntext)])
    destroyevent()
end

function goodevent()
    -- Roll for good event, handle accordingly
    event = rnd.rnd(1, #gtext)

    -- Only give a map if unknown.
    if event == 2 then
        maps = {
            ["Map: Dvaered-Soromid trade route"] = "Dvaered-Soromid trade route",
            ["Map: Sirian border systems"] = "Sirian border systems",
            ["Map: Dvaered Core"] = "Dvaered core systems",
            ["Map: Empire Core"]  = "Empire core systems",
            ["Map: Nebula Edge"]  = "Sol nebula edge",
            ["Map: The Frontier"] = "Frontier systems"
        }
        unknown = {}

        for k,v in pairs(maps) do
            if player.numOutfit(k) == 0 then
                table.insert( unknown, k )
            end
        end

        -- All maps are known.
        if #unknown == 0 then
            while event == 2 do
                event = rnd.rnd(1, #gtext)
            end
        end
    end


    if event == 1 then
        tk.msg(gtitle, gtext[1])
        player.pay(rnd.rnd(5000,30000)) --5K - 30K
    elseif event == 2 then
        local choice = unknown[rnd.rnd(1,#unknown)]
        tk.msg(gtitle, gtext[2]:format(maps[choice]))
        player.addOutfit(choice, 1)
    elseif event == 3 then
        local factions = {"Empire", "Dvaered", "Sirius", "Soromid"} -- TODO: Add more factions as they appear
        rndfact = factions[rnd.rnd(1, #factions)]
        tk.msg(gtitle, string.format(gtext[3], rndfact))
        faction.modPlayerSingle(rndfact, 3)
    end
    destroyevent()
end

function badevent()
    -- Roll for bad event, handle accordingly
    event = rnd.rnd(1, #btext)
    if event == 1 then
        p:hookClear() -- So the pilot doesn't end the event by dying.
        tk.msg(btitle, btext[1])
        p:setHealth(0,0)
        player.pilot():control(true)
        hook.pilot(p, "exploded", "derelict_exploded")
        destroyEvent()
    elseif event == 2 then
        tk.msg(btitle, btext[2])
        player.pilot():setFuel(false)
        destroyevent()
    elseif event == 3 then
        tk.msg(btitle, btext[3])
        v1 = pilot.add("Pirate Vendetta", "pirate", player.pos() + vec2.new( 300, 300))[1]
        v2 = pilot.add("Pirate Vendetta", "pirate", player.pos() + vec2.new(-300, 300))[1]
        a1 = pilot.add("Pirate Ancestor", "pirate", player.pos() + vec2.new(-300,-300))[1]
        a2 = pilot.add("Pirate Ancestor", "pirate", player.pos() + vec2.new( 300,-300))[1]
        v1:control()
        v1:attack(player.pilot())
        v2:control()
        v2:attack(player.pilot())
        a1:control()
        a1:attack(player.pilot())
        a2:control()
        a2:attack(player.pilot())
        destroyevent()
    end
end

function derelict_exploded()
   player.pilot():control(false)
   player.pilot():setHealth(42, 0)
end

function missionevent()
    -- Fetch all missions that haven't been flagged as done yet.
    local mymissions = {}
    
    for i, mission in ipairs(missionlist) do
        if not player.misnDone(mission) then
            mymissions[#mymissions + 1] = mission
        end
    end
    
    -- If no missions are available, default to a neutral event.
    if #mymissions == 0 then
        neutralevent()
    else
        -- Roll a random mission and start it.
        local select = rnd.rnd(1, #mymissions)
        naev.missionStart(mymissions[select])
    
        destroyevent()
    end
end

function destroyevent()
    evt.finish()
end
