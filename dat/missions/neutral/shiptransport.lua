--[[

   MISSION: Ship transporter
   DESCRIPTION: Transport a ship from one Planet to another

  how your ship travel from shipyard to shipyard?
	now you are one of the shiptransporters.
   NOTE: codes from cargo.lua and cargo_common.luashi
--]]


include "jumpdist.lua"
include "numstring.lua"
include "nextjump.lua"

-- Find an inhabited planet 0-3 jumps away.
function cargo_selectMissionDistance ()
    local seed = rnd.rnd()
    if     seed < 0.30 then missdist = 0
    elseif seed < 0.60 then missdist = 1
    elseif seed < 0.80 then missdist = 2
    else                    missdist = 3
    end

    return missdist
end

-- Build a set of target planets
function cargo_selectPlanets(missdist, routepos)
    local planets = {}
    getsysatdistance(system.cur(), missdist, missdist,
        function(s)
            for i, v in ipairs(s:planets()) do
                if v:services()["inhabited"] and v ~= planet.cur() and v:class() ~= 0 and
                        not (s==system.cur() and ( vec2.dist( v:pos(), routepos ) < 2500 ) ) and
                        v:canLand() then
                    planets[#planets + 1] = {v, s}
                end
           end
           return true
        end)

    return planets    
end

-- We have a destination, now we need to calculate how far away it is by simulating the journey there.
-- Assume shortest route with no interruptions.
-- This is used to calculate the reward.
function cargo_calculateDistance(routesys, routepos, destsys, destplanet)
    local traveldist = 0

    while routesys ~= destsys do
        -- We're not in the destination system yet.
        -- So, get the next system on the route, and the distance between our entry point and the jump point to the next system.
        -- Then, set the exit jump point as the next entry point.
        local tempsys = getNextSystem(routesys, destsys)
        local j,r = jump.get( routesys, tempsys )
        traveldist = traveldist + vec2.dist(routepos, j:pos())
        routepos = r:pos()
        routesys = tempsys
    end

    -- We ARE in the destination system now, so route from the entry point to the destination planet.
    traveldist = traveldist + vec2.dist(routepos, destplanet:pos())

    return traveldist
end

function cargo_calculateRoute ()
    origin_p, origin_s = planet.cur()
    local routesys = origin_s
    local routepos = origin_p:pos()
    
    -- Select mission tier.
    local tier = rnd.rnd(0, 4)
    
    -- Farther distances have a lower chance of appearing.
    local missdist = cargo_selectMissionDistance()
    local planets = cargo_selectPlanets(missdist, routepos)
    if #planets == 0 then
       return
    end

    local index      = rnd.rnd(1, #planets)
    local destplanet = planets[index][1]
    local destsys    = planets[index][2]
    
    -- We have a destination, now we need to calculate how far away it is by simulating the journey there.
    -- Assume shortest route with no interruptions.
    -- This is used to calculate the reward.

    local numjumps   = origin_s:jumpDist(destsys)
    local traveldist = cargo_calculateDistance(routesys, routepos, destsys, destplanet)
    
    -- We now know where. But we don't know what yet. Randomly choose a ship type.
    
    local cargoes = {"Ilama", "Hyena", "Schroedinger", "Shark", "Ghwain"}
    local cargo = cargoes[rnd.rnd(1, #cargoes)]

    -- Return lots of stuff
    return destplanet, destsys, numjumps, traveldist, cargo, tier
end


-- Construct the cargo mission description text
function buildCargoMissionDescription( priority, amount, ctype, destplanet, destsys )
    str = "Shipshipment to %s"
    if priority ~= nil then
        str = priority .. " transport to %s"
    end
    if system.cur() ~= destsys then
        str = string.format( "%s in %s", str, destsys:name() )
    end
    return string.format( "%s (%s tonnes)", str:format( destplanet:name()), amount )
end


-- Calculates the minimum possible time taken for the player to reach a destination.
function cargoGetTransit( timelimit, numjumps, traveldist )
    local pstats   = player.pilot():stats()
    local stuperpx = 1 / pstats.speed_max * 30
    local arrivalt = time.get() + time.create(0, 0, traveldist * stuperpx +
            numjumps * pstats.jump_delay + 10180 + 240 * numjumps)
    return arrivalt
end
lang = naev.lang()
if lang == "es" then
else -- default english
    misn_desc = "%s in the %s system needs a delivery of %d tons of %s."
    misn_reward = "%s credits"
    
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

	fship = {}
    fship[1] = "False shipclass"
    fship[2] = "You need a carrier for Shipshipment."

   --=Landing=--

   cargo_land_title = "Delivery success!"

   cargo_land_p1 = {}
   cargo_land_p1[1] = "The Container which contains "
   cargo_land_p1[2] = "The transported "
   cargo_land_p1[3] = "The rest of the brocken "

   cargo_land_p2 = {}
   cargo_land_p2[1] = " is carried out of your ship by a sullen group of workers. The job takes inordinately long to complete, and the leader pays you without speaking a word."
   cargo_land_p2[2] = " is pulled out by a team shortly after you land. Before you can even collect your thoughts, one of them presses a credit chip in your hand and departs."
   cargo_land_p2[3] = " is unloaded by an exhausted-looking bunch of dockworkers. Still, they make fairly good time, delivering your pay upon completion of the job."
   cargo_land_p2[4] = " is unloaded by a team of robotic drones supervised by a human overseer, who hands you your pay when they finish."

    accept_title = "Mission Accepted"
    
    osd_title = "Shiptransport mission"
    osd_msg = "Fly to %s in the %s system."
end

-- Create the mission
function create()
    -- Note: this mission does not make any system claims. 

    -- Calculate the route, distance, jumps and cargo to take
    destplanet, destsys, numjumps, traveldist, cargo, tier = cargo_calculateRoute()
    if destplanet == nil then
       misn.finish(false)
    end
    
    -- Choose amount of cargo and mission reward. This depends on the mission tier.
    -- Note: Pay is independent from amount by design! Not all deals are equally attractive!
    finished_mod = 2.0 -- Modifier that should tend towards 1.0 as naev is finished as a game
    amount = rnd.rnd(25, 50)
    jumpreward = 300
    distreward = 0.09
    reward = 1.5^tier * (numjumps * jumpreward + traveldist * distreward) * finished_mod * (1. + 0.05*rnd.twosigma())
    
    misn.setTitle(buildCargoMissionDescription( nil, amount, cargo, destplanet, destsys ))
    misn.markerAdd(destsys, "computer")
    misn.setDesc(cargosize[tier] .. title_p1[rnd.rnd(1, #title_p1)]:format(destplanet:name(), destsys:name()) .. title_p2:format(cargo, amount, numjumps, traveldist))
    misn.setReward(misn_reward:format(numstring(reward)))
    
end

-- Mission is accepted
function accept()
	if player.pilot():ship():class() ~= "Carrier" then -- False ship-class
        tk.msg(fship[1], fship[2])
        misn.finish()
    end
    if player.pilot():cargoFree() < amount then -- No free cargo space
        tk.msg(full[1], full[2]:format(amount, amount - player.pilot():cargoFree()))
        misn.finish()
    end
    misn.accept()
    misn.cargoAdd(cargo, amount) -- TODO: change to jettisonable cargo once custom commodities are in. For piracy purposes.
    misn.osdCreate(osd_title, {osd_msg:format(destplanet:name(), destsys:name())})
    hook.land("land")
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
