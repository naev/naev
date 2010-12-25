--[[
-- These are rush cargo delivery missions. They can be failed! But, pay is higher to compensate.
-- These missions require fast ships, but higher tiers may also require increased cargo space.
--]]

include "scripts/jumpdist.lua"

lang = naev.lang()
if lang == "es" then
else -- default english
    misn_desc = "%s in the %s system needs a delivery of %d tons of %s."
    misn_reward = "%d credits"
    
    cargosize = {}
    cargosize[0] = "Courier" -- Note: indexed from 0, to match mission tiers.
    cargosize[1] = "Priority"
    cargosize[2] = "Pressing"
    cargosize[3] = "Urgent"
    cargosize[4] = "Emergency"
    
    title_p1 = {}
    title_p1[1] = " cargo delivery to %s in the %s system"
    title_p1[2] = " freight delivery to %s in the %s system"
    title_p1[3] = " transport to %s in the %s system"
    title_p1[4] = " delivery to %s in the %s system"
    
    -- Note: please leave the trailing space on the line below! Needed to make the newline show up.
    title_p2 = [[ 
Cargo: %s (%d tons)
Jumps: %d
Travel distance: %d
Time limit: %s]]

    full = {}
    full[1] = "No room in ship"
    full[2] = "You don't have enough cargo space to accept this mission. You need %d tons of free space (you need %d more)."

	--=Landing=--
	
	cargo_land_title = "Delivery success!"

	cargo_land_p1 = {}
	cargo_land_p1[1] = "The crates of "  --<<-- paired with cargo_accept_p2, don't mix this up!!
	cargo_land_p1[2] = "The drums of "
	cargo_land_p1[3] = "The containers of "

	cargo_land_p2 = {}
	cargo_land_p2[1] = " are carried out of your ship by a sullen group of workers. The job takes inordinately long to complete, and the leader pays you without speaking a word."
	cargo_land_p2[2] = " are rushed out of your vessel by a team shortly after you land. Before you can even collect your thoughts, one of them presses a credit chip in your hand and departs."
	cargo_land_p2[3] = " are unloaded by an exhausted-looking bunch of dockworkers. Still, they make fairly good time, delivering your pay upon completion of the job."

	cargo_land_p3 = {}
	cargo_land_p3[1] = " are carried out of your ship by a sullen group of workers. They are not happy that they have to work overtime because you were late. You are paid only %d of the %d you were promised."
	cargo_land_p3[2] = " are rushed out of your vessel by a team shortly after you land. Your late arrival is stretching quite a few schedules! Your pay is only %d instead of %d because of that."
	cargo_land_p3[3] = " are unloaded by an exhausted-looking bunch of dockworkers. You failed to miss the deadline, so your reward is only %d instead of the %d you were hoping for."

    accept_title = "Mission Accepted"
    
    timeup_1 = "You've missed the deadline for the delivery to %s! But you can still make a late delivery if you hurry."
    timeup_2 = "The delivery to %s has been canceled! You were too late."
    
    osd_title = "Rush cargo mission"
    osd_msg = {}
    osd_msg[1] = "Fly to %s in the %s system before %s."
    osd_msg[2] = "You have %s remaining."
    osd_msg1 = "Fly to %s in the %s system before %s."
    osd_msg2 = "You have %s remaining." -- Need to reuse.
end

-- Create the mission
function create()
    -- Note: this mission does not make any system claims. 
    origin_p, origin_s = planet.cur()
    
    -- Select mission tier.
    local tier = rnd.rnd(0, 4)
    
    -- Find an inhabited planet 0-3 jumps away.
    -- Farther distances have a lower chance of appearing.
    
    local seed = rnd.rnd()
    if     seed < 0.30 then missdist = 0
    elseif seed < 0.60 then missdist = 1
    elseif seed < 0.80 then missdist = 2
    else                    missdist = 3
    end
    
    local planets = {}
    getsysatdistance(system.cur(), missdist, missdist,
        function(s)
            for i, v in ipairs(s:planets()) do
                if v:services()["inhabited"] and v ~= planet.cur() and v:class() ~= 0 then
                    planets[#planets + 1] = {v, s}
                end
           end
           return true
        end)

    if #planets == 0 then
        abort()
    end
    
    index = rnd.rnd(1, #planets)
    destplanet = planets[index][1]
    destsys = planets[index][2]
    
    -- We have a destination, now we need to calculate how far away it is by simulating the journey there.
    -- Assume shortest route with no interruptions.
    -- This is used to calculate the reward, and the time limit.
    local routesys = origin_s
    local routepos = origin_p:pos()
    traveldist = 0
    numjumps = origin_s:jumpDist(destsys)
    
    while routesys ~= destsys do
        -- We're not in the destination system yet.
        -- So, get the next system on the route, and the distance between our entry point and the jump point to the next system.
        -- Then, set the exit jump point as the next entry point.
        local tempsys = getNextSystem(routesys, destsys)
        traveldist = traveldist + vec2.dist(routepos, routesys:jumpPos(tempsys))
        routepos = tempsys:jumpPos(routesys)
        routesys = tempsys
    end
    -- We ARE in the destination system now, so route from the entry point to the destination planet.
    traveldist = traveldist + vec2.dist(routepos, destplanet:pos())
    
    -- We now know where. But we don't know what yet. Randomly choose a commodity type.
    -- TODO: I'm using the standard cargo types for now, but this should be changed to custom cargo once local-defined commodities are implemented.
    local cargoes = {"Food", "Industrial Goods", "Medicine", "Luxury Goods", "Ore"}
    cargo = cargoes[rnd.rnd(1, #cargoes)]
    
    -- Calculate time limit. Depends on tier and distance.
    -- The second time limit is for the reduced reward.
    stuperpx = 0.15 - 0.015 * tier
    stuperjump = 11000 - 1100 * tier
    stupertakeoff = 10000
    timelimit = time.get() + time.create(0, 0, traveldist * stuperpx + numjumps * stuperjump + stupertakeoff)
    timelimit2 = time.get() + time.create(0, 0, (traveldist * stuperpx + numjumps * stuperjump + stupertakeoff) * 1.2)
    
    -- Choose amount of cargo and mission reward. This depends on the mission tier.
    -- Note: Pay is independent from amount by design! Not all deals are equally attractive!
    amount = rnd.rnd(10 + 5 * tier, 20 + 6 * tier) -- 45 max (quicksilver)
    jumpreward = 1000
    distreward = 0.12
    reward = 1.5^tier * (numjumps * jumpreward + traveldist * distreward)
    
    misn.setTitle(cargosize[tier] .. " cargo transport (" .. amount .. " tons of " .. cargo .. ")")
    misn.markerAdd(destsys, "computer")
    misn.setDesc(cargosize[tier] .. title_p1[rnd.rnd(1, #title_p1)]:format(destplanet:name(), destsys:name()) .. title_p2:format(cargo, amount, numjumps, traveldist, (timelimit - time.get()):str()))
    misn.setReward(misn_reward:format(reward))
end

-- Mission is accepted
function accept()
    if player.pilot():cargoFree() < amount then
        tk.msg(full[1], full[2]:format(amount, amount - player.pilot():cargoFree()))
        misn.finish()
    end
    misn.accept()
    intime = true
    misn.cargoAdd(cargo, amount) -- TODO: change to jettisonable cargo once custom commodities are in. For piracy purposes.
    osd_msg[1] = osd_msg1:format(destplanet:name(), destsys:name(), timelimit:str())
    osd_msg[2] = osd_msg2:format((timelimit - time.get()):str())
    misn.osdCreate(osd_title, osd_msg)
    hook.land("land")
    hook.date(time.create(0, 0, 100), "tick") -- 100STU per tick
end

-- Choose the next system to jump to on the route from system nowsys to system finalsys.
function getNextSystem(nowsys, finalsys)
    if nowsys == finalsys then
        return nowsys
    else
        local neighs = nowsys:adjacentSystems()
        local nearest = -1
        local mynextsys = finalsys
        for _, j in pairs(neighs) do
            if nearest == -1 or j:jumpDist(finalsys) < nearest then
                nearest = j:jumpDist(finalsys)
                mynextsys = j
            end
        end
        return mynextsys
    end
end 

-- Land hook
function land()
    if planet.cur() == destplanet then
        if intime then
            -- Semi-random message.
            tk.msg(cargo_land_title, cargo_land_p1[rnd.rnd(1, #cargo_land_p1)] .. cargo .. cargo_land_p2[rnd.rnd(1, #cargo_land_p2)])
        else
            -- Semi-random message for being late.
            tk.msg(cargo_land_title, cargo_land_p1[rnd.rnd(1, #cargo_land_p1)] .. cargo .. cargo_land_p3[rnd.rnd(1, #cargo_land_p3)]:format(reward / 2, reward))
            reward = reward / 2
        end
        player.pay(reward)
        misn.finish(true)
    end
end

-- Date hook
function tick()
    if timelimit >= time.get() then
        -- Case still in time
        osd_msg[1] = osd_msg1:format(destplanet:name(), destsys:name(), timelimit:str())
        osd_msg[2] = osd_msg2:format((timelimit - time.get()):str())
        misn.osdCreate(osd_title, osd_msg)
    elseif intime then
        -- Case missed first deadline
        player.msg(timeup_1:format(destsys:name()))
        osd_msg[1] = osd_msg[1]:format(destplanet:name(), destsys:name(), timelimit:str())
        osd_msg[2] = timeup_1:format(destsys:name())
        misn.osdCreate(osd_title, osd_msg)
        intime = false
    elseif timelimit2 <= time.get() then
        -- Case missed second deadline
        player.msg(timeup_2:format(destsys:name()))
        abort()
    end
end

function abort()
    misn.finish(false)
end
