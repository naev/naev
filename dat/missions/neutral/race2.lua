--[[
--
-- MISSION: Racing Skills 2
-- DESCRIPTION: A man asks you to join a race, where you fly to various checkpoints and board them before landing back at the starting planet
--
--]]

-- Localization, choosing a language if naev is translated for non-english-speaking locales.
lang = naev.lang()
if lang == "es" then
else -- Default to English

    text = {}
    title = {}
    ftitle = {}
    ftext = {}

    title[1] = "Looking for a 4th"
    text[1] = [[    "Hey man, great to see you back.  You want to have another race?"]]   

    title[5] = "Choose difficulty"
    text[5] = [["You can choose two different kinds of races, an easy one, which is like the 1st race we had, or a hard one, with stock ships and smaller checkpoints.  Of course the reward is bigger too. "]]
    title[6] = "Hard Mode"
    text[6] = [["You want a challenge huh?  Remember no outfits are allowed on your Llama or you will not be allowed to race.  Lets go have some fun then."]]
    
    choice1 = "Easy"
    choice2 = "Hard"

    title[2] = "Easy Mode "
    text[2] = [[    "Lets go have some fun then."]]

    title[3] = "Checkpoint %s reached"
    text[3] = "Proceed to Checkpoint %s"

    text[4] = "Proceed to land at %s"
    refusetitle = "Refusal"
    refusetext = [[    "I guess we'll need to find another pilot."]]
    
    wintitle = "You Won!"
    wintext = [[     A man in a suit and tie takes you up onto a stage.  A large nametag on his jacket says 'Melendez Corporation'.  "Congratulations on your win," he says, shaking your hand, "that was a great race.  On behalf of Melendez Corporation, I am here to present to you your prize money."  He hands you one of those fake Oversized Checks with %s credits written on it.]]

    ftitle[1] = "Illegal ship!"
    ftext[1] = [[     "You have switched to a ship that's not allowed in this race.  Mission failed."]]

    ftitle[2] = "You left the race!"
    ftext[2] = [[     "You left the race.  The race continued without you."]]

    ftitle[3] = "You failed to win the race."
    ftext[3] = [[     As you congratulate the winner on a great race, the laid back man comes up to you.
"That was a lot of fun, if you ever have time, lets race again."]]

    ftitle[4] = "Illegal ship!"
    ftext[4] = [[     "You have outfits on your ship which is not allowed in this race in hard mode.  Mission failed."]]

    NPCname = "A laid back man"
    NPCdesc = "You see a laid back man, who appears to be one of the locals, looking around the bar, apparently in search of a suitable pilot."

    misndesc = "You're participating in another race!"
    misnreward = "10000/50000 credits"

    OSDtitle = "Racing Skills 2"
    OSD = {}
    OSD[1] = "Board checkpoint 1"
    OSD[2] = "Board checkpoint 2"
    OSD[3] = "Board checkpoint 3"
    OSD[4] = "Land at %s"

    chatter = {}
    chatter[1] = "Lets do this!"
    chatter[2] = "Wooo!"
    chatter[3] = "Time to Shake 'n Bake"
    chatter[4] = "Checkpoint %s baby!"
    chatter[5] = "Hooyah"
    chatter[6] = "Next!"
    timermsg = "%s"
    target = {1,1,1,1}
    marketing = "This race sponsored by Melendez Corporation.  Have you piloted a Melendez lately?"
    positionmsg = "%s just reached checkpoint %s"
    landmsg = "%s just landed at %s and finished the race"
end


function create ()

    this_planet, this_system = planet.cur()
    missys = {this_system}
    if not misn.claim(missys) then
        misn.finish(false)
    end
    cursys = system.cur()
    curplanet = planet.cur()
    misn.setNPC(NPCname, "neutral/male1")
    misn.setDesc(NPCdesc)


end


function accept ()
    if tk.yesno(title[1], text[1]) then
        misn.accept()
	OSD[4] = string.format(OSD[4], curplanet:name())
        misn.setDesc(misndesc)
        misn.setReward(misnreward)
        misn.osdCreate(OSDtitle, OSD)
        choice, choicetext = tk.choice(title[5], text[5], choice1, choice2)
        if choice == 1 then
            tk.msg(title[2], text[2])
        else
            tk.msg(title[6], text[6])
        end
        hook.takeoff("takeoff")
    else
        tk.msg(refusetitle, refusetext)
        misn.finish()
    end
end

function takeoff()
    if player.pilot():ship():class() ~= "Yacht" then
        tk.msg(ftitle[1], ftext[1])
        abort()
    end
    if choice ~= 1 then
        for k,v in ipairs(player.pilot():outfits()) do
            if (("Manufacturer Small Engine" ~= v:name())
            and ("Basic Small Systems" ~= v:name())
            and ("Unmodified Small Hull" ~= v:name())) then
                tk.msg(ftitle[4], ftext[4])
                abort()
            end
        end
    end
    planetvec = planet.pos(curplanet)
    misn.osdActive(1) 
    checkpoint = {}
    racers = {}
    pilot.toggleSpawn(false)
    pilot.clear()
    dist1 = rnd.rnd() * system.cur():radius()
    angle1 = rnd.rnd() * 2 * math.pi
    location1 = vec2.new(dist1 * math.cos(angle1), dist1 * math.sin(angle1))
    dist2 = rnd.rnd() * system.cur():radius()
    angle2 = rnd.rnd() * 2 * math.pi
    location2 = vec2.new(dist2 * math.cos(angle2), dist2 * math.sin(angle2))
    dist3 = rnd.rnd() * system.cur():radius()
    angle3 = rnd.rnd() * 2 * math.pi
    location3 = vec2.new(dist3 * math.cos(angle3), dist3 * math.sin(angle3))
    if choice == 1 then
        shiptype = "Goddard"
    else
        shiptype = "Koala"
    end
    checkpoint[1] = pilot.addRaw(shiptype, "stationary", location1, "Trader")[1]
    checkpoint[2] = pilot.addRaw(shiptype, "stationary", location2, "Trader")[1]
    checkpoint[3] = pilot.addRaw(shiptype, "stationary", location3, "Trader")[1]
    for i, j in ipairs(checkpoint) do
        j:rename(string.format("Checkpoint %s", i))
        j:control()
        j:setHilight(true)
        j:setInvincible(true)
        j:setActiveBoard(true)
        j:setVisible(true)
    end
    racers[1] = pilot.addRaw("Llama", "soromid", curplanet, "Soromid")[1]
    racers[2] = pilot.addRaw("Llama", "empire", curplanet, "Empire")[1]
    racers[3] = pilot.addRaw("Llama", "dvaered", curplanet, "Dvaered")[1]
    if choice == 1 then
        racers[1]:addOutfit("Engine Reroute")
        racers[2]:addOutfit("Steering Thrusters")
        racers[3]:addOutfit("Improved Stabilizer")
    else
        -- basic loadout
        for i in pairs(racers) do
            racers[i]:rmOutfit("all")
            racers[i]:rmOutfit("cores")
            
            racers[i]:addOutfit("Basic Small Systems")
            racers[i]:addOutfit("Unmodified Small Hull")
            racers[i]:addOutfit("Manufacturer Small Engine")
        end
    end
    for i, j in ipairs(racers) do
        j:rename(string.format("Racer %s", i))
        j:setHilight(true)
        j:setInvincible(true)
        j:setVisible(true)
        j:control()
        j:face(checkpoint[1], true)
        j:broadcast(chatter[i])
    end
    player.pilot():control()
    player.pilot():face(checkpoint[1], true)
    countdown = 5 -- seconds
    omsg = player.omsgAdd(timermsg:format(countdown), 0, 50)
    counting = true
    counterhook = hook.timer(1000, "counter")    
    hook.board("board")
    hook.jumpin("jumpin")
    hook.land("land")
end

function counter()
    countdown = countdown - 1
    if countdown == 0 then
        player.omsgChange(omsg, "Go!", 1000)
        hook.timer(1000, "stopcount")
        player.pilot():control(false)
        counting = false
        hook.rm(counterhook)
        for i, j in ipairs(racers) do
           j:control()
           j:goto(checkpoint[target[i]]:pos())
           hook.pilot(j, "land", "racerland")
        end
	hp1 = hook.pilot(racers[1], "idle", "racer1idle")
	hp2 = hook.pilot(racers[2], "idle", "racer2idle")
	hp3 = hook.pilot(racers[3], "idle", "racer3idle")
        player.msg(marketing)
    else
        player.omsgChange(omsg, timermsg:format(countdown), 0)
        counterhook = hook.timer(1000, "counter")    
    end
end

function racer1idle(p)
    player.msg( string.format( positionmsg, p:name(),target[1]) )
    p:broadcast(string.format( chatter[4], target[1]))
    target[1] = target[1] + 1
    hook.timer(2000, "nexttarget1")
end
function nexttarget1()
    if target[1] == 4 then
        racers[1]:land(curplanet:name())
	hook.rm(hp1)
    else
        racers[1]:goto(checkpoint[target[1]]:pos())
    end
end

function racer2idle(p)
    player.msg( string.format( positionmsg, p:name(),target[2]) )
    p:broadcast(chatter[5])
    target[2] = target[2] + 1
    hook.timer(2000, "nexttarget2")
end
function nexttarget2()
    if target[2] == 4 then
        racers[2]:land(curplanet:name())
        hook.rm(hp2)
    else
        racers[2]:goto(checkpoint[target[2]]:pos())
    end
end
function racer3idle(p)
    player.msg( string.format( positionmsg, p:name(),target[3]) )
    p:broadcast(chatter[6])
    target[3] = target[3] + 1
    hook.timer(2000, "nexttarget3")
end
function nexttarget3()
    if target[3] == 4 then
        racers[3]:land(curplanet:name())
        hook.rm(hp3)
    else
        racers[3]:goto(checkpoint[target[3]]:pos())
    end
end
function stopcount()
    player.omsgRm(omsg)
end
function board(ship)
    for i,j in ipairs(checkpoint) do
        if ship == j and target[4] == i then
            misn.osdActive(i+1)
            target[4] = target[4] + 1
            if target[4] == 4 then
                tk.msg(string.format(title[3], i), string.format(text[4], curplanet:name()))
            else
                tk.msg(string.format(title[3], i), string.format(text[3], i+1))
            end
        end
    end
    player.msg( string.format( positionmsg, player.name(),target[4]) )
    player.unboard()	    
end

function jumpin()
    tk.msg(ftitle[2], ftext[2])
    abort()
end

function racerland(p)
    player.msg( string.format(landmsg, p:name(),curplanet:name()))
end
function land()
    if target[4] == 4 then
        if racers[1]:exists() and racers[2]:exists() and racers[3]:exists() then
            if choice == 1 then
                payment = 10000
            else
                payment = 50000
            end
            tk.msg(wintitle, string.format(wintext,payment))
            player.pay(payment)
            misn.finish(true)
        else
            tk.msg(ftitle[3], ftext[3])
            abort()
        end
    else
        tk.msg(ftitle[2], ftext[2])
        abort()
    end
end

function abort ()
    misn.finish(false)
end
