--[[
-- These are regular cargo delivery missions. Pay is low, but so is difficulty.
-- Most of these missions require BULK ships. Not for small ships!
--]]

include "dat/scripts/cargo_common.lua"
include "dat/scripts/numstring.lua"

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
Travel distance: %d
Piracy Risk: %s]]

    full = {}
    full[1] = "No room in ship"
    full[2] = "You don't have enough cargo space to accept this mission. You need %d tons of free space (you need %d more)."

	piracyrisk = {}
	piracyrisk[1] = "None"
	piracyrisk[2] = "Low"
	piracyrisk[3] = "Medium"
	piracyrisk[4] = "High"
   --=Landing=--

   cargo_land_title = "Delivery success!"

   cargo_land_p1 = {}
   cargo_land_p1[1] = "The crates of "
   cargo_land_p1[2] = "The drums of "
   cargo_land_p1[3] = "The containers of "

   cargo_land_p2 = {}
   cargo_land_p2[1] = " are carried out of your ship by a sullen group of workers. The job takes inordinately long to complete, and the leader pays you without speaking a word."
   cargo_land_p2[2] = " are rushed out of your vessel by a team shortly after you land. Before you can even collect your thoughts, one of them presses a credit chip in your hand and departs."
   cargo_land_p2[3] = " are unloaded by an exhausted-looking bunch of dockworkers. Still, they make fairly good time, delivering your pay upon completion of the job."
   cargo_land_p2[4] = " are unloaded by a team of robotic drones supervised by a human overseer, who hands you your pay when they finish."

    accept_title = "Mission Accepted"
    
    osd_title = "Cargo mission"
    osd_msg = "Fly to %s in the %s system."
end

-- Create the mission
function create()
    -- Note: this mission does not make any system claims. 

    -- Calculate the route, distance, jumps, risk of piracy, and cargo to take
    destplanet, destsys, numjumps, traveldist, cargo, avgrisk, tier = cargo_calculateRoute()
    
    if destplanet == nil then
       misn.finish(false)
    end
    
    if avgrisk == 0 then
       piracyrisk = piracyrisk[1]
       riskreward = 0
    elseif avgrisk <= 25 then
       piracyrisk = piracyrisk[2]
       riskreward = 10
    elseif avgrisk > 25 and avgrisk <= 100 then
       piracyrisk = piracyrisk[3]
       riskreward = 25
    else
       piracyrisk = piracyrisk[4]
       riskreward = 50
    end
       
    -- Choose amount of cargo and mission reward. This depends on the mission tier.
    -- Reward depends on type of cargo hauled. Hauling expensive commodities gives a better deal.
    -- Note: Pay is independent from amount by design! Not all deals are equally attractive!
    finished_mod = 2.0 -- Modifier that should tend towards 1.0 as naev is finished as a game
    amount = rnd.rnd(5 + 25 * tier, 20 + 60 * tier)
    jumpreward = commodity.price(cargo)
    distreward = math.log(100*commodity.price(cargo))/100
    reward = 1.5^tier * (avgrisk*riskreward + numjumps * jumpreward + traveldist * distreward) * finished_mod * (1. + 0.05*rnd.twosigma())
    
    misn.setTitle(buildCargoMissionDescription( nil, amount, cargo, destplanet, destsys ))
    misn.markerAdd(destsys, "computer")
    misn.setDesc(cargosize[tier] .. title_p1[rnd.rnd(1, #title_p1)]:format(destplanet:name(), destsys:name()) .. title_p2:format(cargo, amount, numjumps, traveldist, piracyrisk))
    misn.setReward(misn_reward:format(numstring(reward)))
    
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

