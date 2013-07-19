--[[
-- These are rush cargo delivery missions. They can be failed! But, pay is higher to compensate.
-- These missions require fast ships, but higher tiers may also require increased cargo space.
--]]

include "jumpdist.lua"
include "cargo_common.lua"
include "numstring.lua"

lang = naev.lang()
if lang == "es" then
else -- default english
    misn_desc = "%s in the %s system needs a delivery of %d tonnes of %s."
    misn_reward = "%s credits"
    
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
Cargo: %s (%d tonnes)
Jumps: %d
Travel distance: %d
Time limit: %s]]

    full = {}
    full[1] = "No room in ship"
    full[2] = "You don't have enough cargo space to accept this mission. It requires %d tonnes of free space (you need %d more)."

   slow = {}
   slow[1] = "Too slow"
   slow[2] = [[This shipment must arrive within %s, but it will take at least %s for your ship to reach %s, missing the deadline.

Accept the mission anyway?]]

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
   cargo_land_p3[3] = " are unloaded by an exhausted-looking bunch of dockworkers. You missed the deadline, so your reward is only %d instead of the %d you were hoping for."

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

    -- Calculate the route, distance, jumps and cargo to take
    destplanet, destsys, numjumps, traveldist, cargo, tier = cargo_calculateRoute()
    if destplanet == nil then
       misn.finish(false)
    end
    
    -- Calculate time limit. Depends on tier and distance.
    -- The second time limit is for the reduced reward.
    stuperpx   = 0.2 - 0.025 * tier
    stuperjump = 10300 - 300 * tier
    stupertakeoff = 10300 - 75 * tier
    timelimit  = time.get() + time.create(0, 0, traveldist * stuperpx + numjumps * stuperjump + stupertakeoff + 240 * numjumps)
    timelimit2 = time.get() + time.create(0, 0, (traveldist * stuperpx + numjumps * stuperjump + stupertakeoff + 240 * numjumps) * 1.2)

    -- Choose amount of cargo and mission reward. This depends on the mission tier.
    -- Note: Pay is independent from amount by design! Not all deals are equally attractive!
    finished_mod = 2.0 -- Modifier that should tend towards 1.0 as naev is finished as a game
    amount     = rnd.rnd(10 + 5 * tier, 20 + 6 * tier) -- 45 max (quicksilver)
    jumpreward = 1000
    distreward = 0.12
    reward     = 1.5^tier * (numjumps * jumpreward + traveldist * distreward) * finished_mod * (1. + 0.05*rnd.twosigma())

    misn.setTitle( buildCargoMissionDescription(cargosize[tier], amount, cargo, destplanet, destsys ))
    misn.markerAdd(destsys, "computer")
    misn.setDesc(cargosize[tier] .. title_p1[rnd.rnd(1, #title_p1)]:format(destplanet:name(), destsys:name()) .. title_p2:format(cargo, amount, numjumps, traveldist, (timelimit - time.get()):str()))
    misn.setReward(misn_reward:format(numstring(reward)))
end

-- Mission is accepted
function accept()
    if player.pilot():cargoFree() < amount then
        tk.msg(full[1], full[2]:format(amount, amount - player.pilot():cargoFree()))
        misn.finish()
    end
    pilot.cargoAdd( player.pilot(), cargo, amount ) 
    local playerbest = cargoGetTransit( timelimit, numjumps, traveldist )
    pilot.cargoRm( player.pilot(), cargo, amount ) 
    if timelimit < playerbest then
        if not tk.yesno( slow[1], slow[2]:format( (timelimit - time.get()):str(), (playerbest - time.get()):str(), destplanet:name()) ) then
            misn.finish()
        end
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
    elseif timelimit2 <= time.get() then
        -- Case missed second deadline
        player.msg(timeup_2:format(destsys:name()))
        abort()
    elseif intime then
        -- Case missed first deadline
        player.msg(timeup_1:format(destsys:name()))
        osd_msg[1] = osd_msg[1]:format(destplanet:name(), destsys:name(), timelimit:str())
        osd_msg[2] = timeup_1:format(destsys:name())
        misn.osdCreate(osd_title, osd_msg)
        intime = false
    end
end

function abort()
    misn.finish(false)
end
