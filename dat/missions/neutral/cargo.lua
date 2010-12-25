--[[
-- These are regular cargo delivery missions. Pay is low, but so is difficulty.
-- Most of these missions require BULK ships. Not for small ships!
--]]

include "scripts/jumpdist.lua"

lang = naev.lang()
if lang == "es" then
else -- default english
    misn_desc = "%s in the %s system needs a delivery of %d tons of %s."
    misn_reward = "%d credits"
    
    cargosize = {}
    cargosize[0] = "Small" -- Note: indexed from 0, to match mission tiers.
    cargosize[1] = "Medium"
    cargosize[2] = "Sizeable"
    cargosize[3] = "Large"
    cargosize[4] = "Bulk"
    
    title_p1 = {}
    title_p1[1] = " cargo delivery to %s in the %s system"
    title_p1[2] = " freight delivery to %s in the %s system"
    title_p1[3] = " transport to %s in the %s system"
    title_p1[4] = " delivery to %s in the %s system"
    
    -- Note: please leave the trailing space on the line below! Needed to make the newline show up.
    title_p2 = [[ 
Cargo: %s (%d tons)
Jumps: %d
Travel distance: %d]]

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

    accept_title = "Mission Accepted"
    
    osd_title = "Cargo mission"
    osd_msg = "Fly to %s in the %s system."
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
    -- This is used to calculate the reward.
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
    
    
    -- Choose amount of cargo and mission reward. This depends on the mission tier.
    -- Note: Pay is independent from amount by design! Not all deals are equally attractive!
    amount = rnd.rnd(5 + 25 * tier, 20 + 60 * tier)
    jumpreward = 200
    distreward = 0.09
    reward = 1.5^tier * (numjumps * jumpreward + traveldist * distreward)
    
    misn.setTitle("Cargo transport (" .. amount .. " tons of " .. cargo .. ")")
    misn.markerAdd(destsys, "computer")
    misn.setDesc(cargosize[tier] .. title_p1[rnd.rnd(1, #title_p1)]:format(destplanet:name(), destsys:name()) .. title_p2:format(cargo, amount, numjumps, traveldist))
    misn.setReward(misn_reward:format(reward))
    
end

-- Mission is accepted
function accept()
    if player.pilot():cargoFree() < amount then
        tk.msg(full[1], full[2]:format(amount, amount - player.pilot():cargoFree()))
        misn.finish()
    end
    misn.accept()
    misn.cargoAdd(cargo, amount) -- TODO: change to jettisonable cargo once custom commodities are in. For piracy purposes.
    misn.osdCreate(osd_title, {osd_msg:format(destplanet:name(), destsys:name())})
    hook.land("land")
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
        -- Semi-random message.
        tk.msg(cargo_land_title, cargo_land_p1[rnd.rnd(1, #cargo_land_p1)] .. cargo .. cargo_land_p2[rnd.rnd(1, #cargo_land_p2)])
        player.pay(reward)
        misn.finish(true)
    end
end

function abort ()
    misn.finish(false)
end

