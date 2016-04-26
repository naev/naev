--Escort a convoy of traders to a destination--

include "dat/scripts/nextjump.lua"
include "dat/scripts/cargo_common.lua"
include "dat/scripts/numstring.lua"

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english

    misn_title = "Escort a %s convoy to %s in %s."
    misn_reward = "%s credits"
    
    convoysize = {}
    convoysize[1] = "tiny"
    convoysize[2] = "small" 
    convoysize[3] = "medium"
    convoysize[4] = "large"
    convoysize[5] = "huge"
    
    title_p1 = "A %s convoy of traders needs protection to %s in %s. You must stick with the convoy at all times, waiting to jump or land until the entire convoy has done so."
    
    -- Note: please leave the trailing space on the line below! Needed to make the newline show up.
    title_p2 = [[ 
    
Cargo: %s
Jumps: %d
Travel distance: %d
Piracy Risk: %s]]

	piracyrisk = {}
	piracyrisk[1] = "None"
	piracyrisk[2] = "Low"
	piracyrisk[3] = "Medium"
	piracyrisk[4] = "High"
   
    accept_title = "Mission Accepted"
    
    osd_title = "Convey Escort"
    osd_msg = "Escort a convoy of traders to %s in the %s system."
	
   	slow = {}
  	slow[1] = "Not enough fuel"
  	slow[2] = [[The destination is %s jumps away, but you only have enough fuel for %s jumps. You cannot stop to refuel.

Accept the mission anyway?]]

    landsuccesstitle = "Success!"
    landsuccesstext = "You successfully escorted the trading convoy to the destination. There wasn't a single casualty, and you are rewarded the full amount."
    
    landcasualtytitle = "Success with Casualities"
    landcasualtytext = {}
    landcasualtytext[1] = "You've arrived with the trading convoy more or less intact. Your pay is docked slightly to provide compensation for the families of the lost crew members."
    landcasualtytext[2] = "You arrive with what's left of the convoy. It's not much, but it's better than nothing. After a moment of silence of the lost crew members, you are paid a steeply discounted amount."

    wrongsystitle = "You diverged!"
    wrongsystext = [[You have jumped to the wrong system! You are no longer part of the mission to escort the traders.]]
    
    convoydeathtitle = "The convoy has been destroyed!"
    convoydeathtext = [[All of the traders have been killed. You are a terrible escort.]]
    
    landfailtitle = "You abandoned your mission!"
    landfailtext = "You have landed, but you were supposed to escort the trading convoy. Your mission is a failure!"
    
    
    convoynoruntitle = "You have left your convoy behind!"
    convoynoruntext = [[You jumped before the rest of your convoy and the remaining ships scatter to find cover. You have abandoned your duties, and failed your mission.]]
    
    convoynolandtitle = "You landed before the convoy!"
    convoynolandtext = [[You landed at the planet before ensuring that the rest of your convoy was safe. You have abandoned your duties, and failed your mission.]]
    
	traderdistress = "Convoy ships under attack! Requesting immediate assistance!"
end


function create()
	--This mission does not make any system claims
    destplanet, destsys, numjumps, traveldist, cargo, avgrisk, tier = cargo_calculateRoute()
    
    if destplanet == nil then
       misn.finish(false)
    elseif numjumps == 0 then
       misn.finish(false) -- have to escort them at least one jump!
    elseif avgrisk <= 25 then
       misn.finish(false) -- needs to be a little bit of piracy possible along route
    end
    
    if avgrisk > 25 and avgrisk <= 100 then
       piracyrisk = piracyrisk[3]
       riskreward = 25
    else
       piracyrisk = piracyrisk[4]
       riskreward = 50
    end
    
    --Select size of convoy. Must have enough combat exp to get large convoys.
	if player.getRating() >= 5000 then
		convoysize = convoysize[rnd.rnd(1,5)]
	elseif player.getRating() >= 2000 then
		convoysize = convoysize[rnd.rnd(1,4)]
	elseif player.getRating() >= 1000 then
		convoysize = convoysize[rnd.rnd(1,3)]
	elseif player.getRating() >= 500 then
		convoysize = convoysize[rnd.rnd(1,2)]
	else
		convoysize = convoysize[1]
	end
	
    -- Choose mission reward. This depends on the mission tier.
    -- Reward depends on type of cargo hauled. Hauling expensive commodities gives a better deal.
    finished_mod = 2.0 -- Modifier that should tend towards 1.0 as naev is finished as a game
    if convoysize == "tiny" then
		tier = 1
		jumpreward = commodity.price(cargo)
		distreward = math.log(100*commodity.price(cargo))/100
    elseif convoysize == "small" then
		tier = 2
		jumpreward = commodity.price(cargo)
		distreward = math.log(250*commodity.price(cargo))/100
    elseif convoysize == "medium" then
		tier = 3
		jumpreward = commodity.price(cargo)
		distreward = math.log(500*commodity.price(cargo))/100
    elseif convoysize == "large" then
		tier = 4
		jumpreward = commodity.price(cargo)
		distreward = math.log(1000*commodity.price(cargo))/100
	elseif convoysize == "huge" then
		tier = 5
		jumpreward = commodity.price(cargo)
		distreward = math.log(2000*commodity.price(cargo))/100
	end
    reward = 2.0^tier * (avgrisk * riskreward + numjumps * jumpreward + traveldist * distreward) * finished_mod * (1. + 0.05*rnd.twosigma())
    
    misn.setTitle( misn_title:format( convoysize, destplanet:name(), destsys:name() ) )
    misn.setDesc(title_p1:format(convoysize, destplanet:name(), destsys:name()) .. title_p2:format(cargo, numjumps, traveldist, piracyrisk))
    misn.markerAdd(destsys, "computer")
    misn.setReward(misn_reward:format(numstring(reward)))
    
end

function accept()
    
	if convoysize == "tiny" then
		convoyname = "Trader Convoy 3"
		alive = {true, true, true} -- Keep track of the traders. Update this when they die.
    	alive["__save"] = true
	elseif convoysize == "small" then
		convoyname = "Trader Convoy 4"
		alive = {true, true, true, true}
    	alive["__save"] = true
	elseif convoysize == "medium" then
		convoyname = "Trader Convoy 5"
		alive = {true, true, true, true, true}
    	alive["__save"] = true
	elseif convoysize == "large" then
		convoyname = "Trader Convoy 6"
		alive = {true, true, true, true, true, true}
    	alive["__save"] = true
	elseif convoysize == "huge" then
		convoyname = "Trader Convoy 8"
		alive = {true, true, true, true, true, true, true, true}
    	alive["__save"] = true		
	end
 
    if player.jumps() < numjumps then
        if not tk.yesno( slow[1], slow[2]:format( numjumps, player.jumps())) then
            misn.finish()
        end
    end
    
    nextsys = getNextSystem(system.cur(), destsys) -- This variable holds the system the player is supposed to jump to NEXT.
    origin = planet.cur() -- The place where the AI ships spawn from.
	
	misnfail = false

    misn.accept()
    misn.osdCreate(osd_title, {osd_msg:format(destplanet:name(), destsys:name())})
    
    hook.takeoff("takeoff")
    hook.jumpin("jumpin")
    hook.jumpout("jumpout")
    hook.land("land")


end

function takeoff()
	--Make it interesting
	if convoysize == "tiny" then
		ambush = pilot.add("Trader Ambush 1", "baddie_norun", vec2.new(0, 0))
	elseif convoysize == "small" then
		ambush = pilot.add(string.format("Trader Ambush %i", rnd.rnd(1,2)), "baddie_norun", vec2.new(0, 0))
	elseif convoysize == "medium" then
		ambush = pilot.add(string.format("Trader Ambush %i", rnd.rnd(2,3)), "baddie_norun", vec2.new(0, 0))
	elseif convoysize == "large" then
		ambush = pilot.add(string.format("Trader Ambush %i", rnd.rnd(2,4)), "baddie_norun", vec2.new(0, 0))
	else
		ambush = pilot.add(string.format("Trader Ambush %i", rnd.rnd(3,5)), "baddie_norun", vec2.new(0, 0))
	end
    --Spawn the convoy
    convoy = pilot.add(convoyname, nil, origin)
    for i, j in ipairs(convoy) do
    	if not alive[i] then j:rm() end -- Dead trader stays dead.
        if j:exists() then
            j:control()
            j:setHilight(true)
            j:setInvincPlayer()
            j:hyperspace(getNextSystem(system.cur(), destsys))
            n = pilot.cargoFree(j)
            c = pilot.cargoAdd(j, cargo, n)
            convoyJump = false
            hook.pilot(j, "death", "traderDeath")
            hook.pilot(j, "jump", "traderJump")
            hook.pilot(j, "attacked", "traderAttacked", j)
        end
    end
end

function jumpin()
    
    if system.cur() ~= nextsys then -- player jumped to somewhere other than the next system
        tk.msg(wrongsystitle, wrongsystext)
        abort()
    elseif system.cur() == destsys and misnfail == false then -- player has reached the destination system
		--Make it interesting
		if convoysize == "tiny" then
			ambush = pilot.add("Trader Ambush 1", "baddie_norun", vec2.new(0, 0))
		elseif convoysize == "small" then
			ambush = pilot.add(string.format("Trader Ambush %i", rnd.rnd(1,2)), "baddie_norun", vec2.new(0, 0))
		elseif convoysize == "medium" then
			ambush = pilot.add(string.format("Trader Ambush %i", rnd.rnd(2,3)), "baddie_norun", vec2.new(0, 0))
		elseif convoysize == "large" then
			ambush = pilot.add(string.format("Trader Ambush %i", rnd.rnd(2,4)), "baddie_norun", vec2.new(0, 0))
		else
			ambush = pilot.add(string.format("Trader Ambush %i", rnd.rnd(3,5)), "baddie_norun", vec2.new(0, 0))
		end
    	--Spawn the convoy
 		convoy = pilot.add(convoyname, nil, origin)  
    	for i, j in ipairs(convoy) do
            if not alive[i] then j:rm() end -- Dead traders stay dead.
            if j:exists() then
                j:control()
                j:setHilight(true)
                j:setInvincPlayer()
                j:land(destplanet)
                n = pilot.cargoFree(j)
                c = pilot.cargoAdd(j, cargo, n)
                convoyLand = false
                hook.pilot(j, "death", "traderDeath")
                hook.pilot(j, "land", "traderLand")
                hook.pilot(j, "attacked", "traderAttacked", j)
            end
    	end
    elseif misnfail == false then -- Not yet at destination, so traders continue to next system.
		--Make it interesting
		if convoysize == "tiny" then
			ambush = pilot.add("Trader Ambush 1", "baddie_norun", vec2.new(0, 0))
		elseif convoysize == "small" then
			ambush = pilot.add(string.format("Trader Ambush %i", rnd.rnd(1,2)), "baddie_norun", vec2.new(0, 0))
		elseif convoysize == "medium" then
			ambush = pilot.add(string.format("Trader Ambush %i", rnd.rnd(2,3)), "baddie_norun", vec2.new(0, 0))
		elseif convoysize == "large" then
			ambush = pilot.add(string.format("Trader Ambush %i", rnd.rnd(2,4)), "baddie_norun", vec2.new(0, 0))
		else
			ambush = pilot.add(string.format("Trader Ambush %i", rnd.rnd(3,5)), "baddie_norun", vec2.new(0, 0))
		end
 		--Spawn the convoy
 		convoy = pilot.add(convoyname, nil, origin)  
    	for i, j in ipairs(convoy) do
            if not alive[i] then j:rm() end -- Dead traders stay dead.
            if j:exists() then
                j:control()
                j:setHilight(true)
                j:setInvincPlayer()
                j:hyperspace(getNextSystem(system.cur(), destsys))
                convoyJump = false
                n = pilot.cargoFree(j)
                c = pilot.cargoAdd(j, cargo, n)
                hook.pilot(j, "death", "traderDeath")
                hook.pilot(j, "jump", "traderJump")
                hook.pilot(j, "attacked", "traderAttacked", j)
            end
    	end
    elseif misnfail == true then -- Jumped ahead of traders. Convoy appears in next system, but reverts to AI
 		hook.timer(4000, "convoyContinues") -- waits 4 secs then has them follow.
    end
end

function jumpout()
    if not convoyJump then
        tk.msg(convoynoruntitle, convoynoruntext)
        misnfail = true
    end
    origin = system.cur()
    nextsys = getNextSystem(system.cur(), destsys)
end

function land()
	dead = 0
	for k,v in pairs(alive) do
		if v ~= true then
		dead = dead + 1
		end
	end
	if planet.cur() ~= destplanet then
		tk.msg(landfailtitle, landfailtext)
		misn.finish(false)
	elseif planet.cur() == destplanet and convoyLand == false then
		tk.msg(convoynolandtitle, convoynolandtext)
		misn.finish(false)
	elseif planet.cur() == destplanet and convoyLand == true and convoysize == "tiny" then
		if dead == 0 then
			tk.msg(landsuccesstitle, landsuccesstext)
			player.pay(reward)
			misn.finish(true)
		elseif dead == 1 then
			tk.msg(landcasualtytitle, landcasualtytext[1])
			player.pay(reward/2)
			misn.finish(true)
		elseif dead == 2 then
			tk.msg(landcasualtytitle, landcasualtytext[2])
			player.pay(reward/4)
			misn.finish(true)
		end
	elseif planet.cur() == destplanet and convoyLand == true and convoysize == "small" then
		if dead == 0 then
			tk.msg(landsuccesstitle, landsuccesstext)
			player.pay(reward)
			misn.finish(true)
		elseif dead > 0 and dead < 3 then
			tk.msg(landcasualtytitle, landcasualtytext[1])
			player.pay(reward/2)
			misn.finish(true)
		elseif dead == 3 then
			tk.msg(landcasualtytitle, landcasualtytext[2])
			player.pay(reward/4)
			misn.finish(true)
		end
	elseif planet.cur() == destplanet and convoyLand == true and convoysize == "medium" then
		if dead == 0 then
			tk.msg(landsuccesstitle, landsuccesstext)
			player.pay(reward)
			misn.finish(true)
		elseif dead > 0 and dead < 3 then
			tk.msg(landcasualtytitle, landcasualtytext[1])
			player.pay(reward/2)
			misn.finish(true)
		elseif dead >= 3 then
			tk.msg(landcasualtytitle, landcasualtytext[2])
			player.pay(reward/4)
			misn.finish(true)
		end
	elseif planet.cur() == destplanet and convoyLand == true and convoysize == "large" then
		if dead == 0 then
			tk.msg(landsuccesstitle, landsuccesstext)
			player.pay(reward)
			misn.finish(true)
		elseif dead <= 3 then
			tk.msg(landcasualtytitle, landcasualtytext[1])
			player.pay(reward/2)
			misn.finish(true)
		elseif dead > 3 then
			tk.msg(landcasualtytitle, landcasualtytext[2])
			player.pay(reward/4)
			misn.finish(true)
		end
	elseif planet.cur() == destplanet and convoyLand == true and convoysize == "huge" then
		if dead == 0 then
			tk.msg(landsuccesstitle, landsuccesstext)
			player.pay(reward)
			misn.finish(true)
		elseif dead <= 4 then
			tk.msg(landcasualtytitle, landcasualtytext[1])
			player.pay(reward/2)
			misn.finish(true)
		elseif dead > 4 then
			tk.msg(landcasualtytitle, landcasualtytext[2])
			player.pay(reward/4)
			misn.finish(true)
		end
	end
end

function traderDeath()
	if convoysize == "tiny" then
    	if alive[3] then alive[3] = false
    	elseif alive[2] then alive[2] = false
    	else -- all convoy dead
        	tk.msg(convoydeathtitle, convoydeathtext)
        	abort()
    	end
	elseif convoysize == "small" then
    	if alive[4] then alive[4] = false
    	elseif alive[3] then alive[3] = false
    	elseif alive[2] then alive[2] = false
    	else -- all convoy dead
        	tk.msg(convoydeathtitle, convoydeathtext)
        	abort()
    	end
	elseif convoysize == "medium" then
    	if alive[5] then alive[5] = false
    	elseif alive[4] then alive[4] = false
    	elseif alive[3] then alive[3] = false
    	elseif alive[2] then alive[2] = false
    	elseif alive[1] then alive[1] = false
    	else -- all convoy dead
        	tk.msg(convoydeathtitle, convoydeathtext)
        	abort()
    	end
    elseif convoysize == "large" then
    	if alive[6] then alive[6] = false
    	elseif alive[5] then alive[5] = false
    	elseif alive[4] then alive[4] = false
    	elseif alive[3] then alive[3] = false
    	elseif alive[2] then alive[2] = false
    	else -- all convoy dead
        	tk.msg(convoydeathtitle, convoydeathtext)
        	abort()
    	end
    elseif convoysize == "huge" then
    	if alive[8] then alive[8] = false
    	elseif alive[7] then alive[7] = false
    	elseif alive[6] then alive[6] = false
    	elseif alive[5] then alive[5] = false
    	elseif alive[4] then alive[4] = false
    	elseif alive[3] then alive[3] = false
    	elseif alive[2] then alive[2] = false
    	else -- all convoy dead
        	tk.msg(convoydeathtitle, convoydeathtext)
        	abort()
    	end
    end
end

-- Handle the jumps of convoy.
function traderJump()
	convoyJump = true
	local broadcast = false
	for i, j in pairs(convoy) do
        if j:exists() and broadcast == false then
			player.msg(string.format("%s has jumped to %s.", j:name(),getNextSystem(system.cur(), destsys):name()))
			broadcast = true
		end
	end
end

--Handle landing of convoy
function traderLand()
	convoyLand = true
	local broadcast = false
	for i, j in pairs(convoy) do
        if j:exists() and broadcast == false then
			player.msg(string.format("%s has landed on %s.", j:name(),destplanet:name()))
			broadcast = true
		end
	end
end


-- Handle the convoy getting attacked.
function traderAttacked(j)
    if shuttingup == true then return
    else
        shuttingup = true
        j:comm(player.pilot(),traderdistress)
        hook.timer(5000, "traderShutup") -- Shuts him up for at least 5s.
    end
end

function traderShutup()
    shuttingup = false
end

function convoyContinues()
 	convoy = pilot.add(convoyname, nil, origin) 
    for i, j in ipairs(convoy) do
    	if not alive[i] then j:rm() end -- Dead traders stay dead.
        if j:exists() then
            j:changeAI( "trader" )
            j:control(false)
        end
    end 
	misn.finish(false)
end