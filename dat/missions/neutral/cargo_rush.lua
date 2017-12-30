--[[
   -- These are rush cargo delivery missions. They can be failed! But, pay is higher to compensate.
   -- These missions require fast ships, but higher tiers may also require increased cargo space.
--]]

include "dat/scripts/cargo_common.lua"
include "dat/scripts/numstring.lua"

misn_desc = _("%s in the %s system needs a delivery of %d tonnes of %s.")
misn_reward = _("%s credits")

cargosize = {}
cargosize[0] = _("Courier") -- Note: indexed from 0, to match mission tiers.
cargosize[1] = _("Priority")
cargosize[2] = _("Pressing")
cargosize[3] = _("Urgent")
cargosize[4] = _("Emergency")

title_p1 = {}
title_p1[1] = _(" cargo delivery to %s in the %s system")
title_p1[2] = _(" freight delivery to %s in the %s system")
title_p1[3] = _(" transport to %s in the %s system")
title_p1[4] = _(" delivery to %s in the %s system")

title_p2 = _([[ 
      Cargo: %s (%d tonnes)
      Jumps: %d
      Travel distance: %d
      Piracy Risk: %s
      Time limit: %s
]])
   
full = {}
full[1] = _("No room in ship")
full[2] = _("You don't have enough cargo space to accept this mission. It requires %d tonnes of free space (you need %d more).")

slow = {}
slow[1] = _("Too slow")
slow[2] = _([[This shipment must arrive within %s, but it will take at least %s for your ship to reach %s, missing the deadline.
   
Accept the mission anyway?]])

piracyrisk = {}
piracyrisk[1] = _("None")
piracyrisk[2] = _("Low")
piracyrisk[3] = _("Medium")
piracyrisk[4] = _("High")

--=Landing=--

cargo_land_title = _("Delivery success!")

cargo_land_p1 = {}
cargo_land_p1[1] = _("The crates of ")  --<<-- paired with cargo_accept_p2, don't mix this up!!
cargo_land_p1[2] = _("The drums of ")
cargo_land_p1[3] = _("The containers of ")

cargo_land_p2 = {}
cargo_land_p2[1] = _("%s%s are carried out of your ship by a sullen group of workers. The job takes inordinately long to complete, and the leader pays you without speaking a word.")
cargo_land_p2[2] = _("%s%s are rushed out of your vessel by a team shortly after you land. Before you can even collect your thoughts, one of them presses a credit chip in your hand and departs.")
cargo_land_p2[3] = _("%s%s are unloaded by an exhausted-looking bunch of dockworkers. Still, they make fairly good time, delivering your pay upon completion of the job.")

cargo_land_p3 = {}
cargo_land_p3[1] = _("%s%s are carried out of your ship by a sullen group of workers. They are not happy that they have to work overtime because you were late. You are paid only %d of the %d you were promised.")
cargo_land_p3[2] = _("%s%s are rushed out of your vessel by a team shortly after you land. Your late arrival is stretching quite a few schedules! Your pay is only %d instead of %d because of that.")
cargo_land_p3[3] = _("%s%s are unloaded by an exhausted-looking bunch of dockworkers. You missed the deadline, so your reward is only %d instead of the %d you were hoping for.")

accept_title = _("Mission Accepted")

timeup_1 = _("You've missed the deadline for the delivery to %s! But you can still make a late delivery if you hurry.")
timeup_2 = _("The delivery to %s has been canceled! You were too late.")

osd_title = _("Rush cargo mission")
osd_msg = {}
osd_msg[1] = _("Fly to %s in the %s system before %s.")
osd_msg[2] = _("You have %s remaining.")
osd_msg1 = _("Fly to %s in the %s system before %s.")
osd_msg2 = _("You have %s remaining.") -- Need to reuse.

-- Create the mission
function create()
   -- Note: this mission does not make any system claims. 
   
   -- Calculate the route, distance, jumps, risk of piracy, and cargo to take
   destplanet, destsys, numjumps, traveldist, cargo, avgrisk, tier = cargo_calculateRoute()
   if destplanet == nil then
      misn.finish(false)
   end
   
   -- Calculate time limit. Depends on tier and distance.
   -- The second time limit is for the reduced reward.
   stuperpx   = 0.2 - 0.025 * tier
   stuperjump = 10300 - 300 * tier
   stupertakeoff = 10300 - 75 * tier
   allowance  = traveldist * stuperpx + numjumps * stuperjump + stupertakeoff + 240 * numjumps
   
   -- Allow extra time for refuelling stops.
   local jumpsperstop = 3 + math.min(tier, 3)
   if numjumps > jumpsperstop then
      allowance = allowance + math.floor((numjumps-1) / jumpsperstop) * stuperjump
   end
   
   timelimit  = time.get() + time.create(0, 0, allowance)
   timelimit2 = time.get() + time.create(0, 0, allowance * 1.2)
   
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
   -- Note: Pay is independent from amount by design! Not all deals are equally attractive!
   finished_mod = 2.0 -- Modifier that should tend towards 1.0 as naev is finished as a game
   amount     = rnd.rnd(10 + 5 * tier, 20 + 6 * tier) -- 45 max (quicksilver)
   jumpreward = commodity.price(cargo)*1.2
   distreward = math.log(300*commodity.price(cargo))/100
   reward     = 1.5^tier * (avgrisk*riskreward + numjumps * jumpreward + traveldist * distreward) * finished_mod * (1. + 0.05*rnd.twosigma())
   
   misn.setTitle( buildCargoMissionDescription(cargosize[tier], amount, cargo, destplanet, destsys ))
   misn.markerAdd(destsys, "computer")
   misn.setDesc(cargosize[tier] .. title_p1[rnd.rnd(1, #title_p1)]:format(destplanet:name(), destsys:name()) .. title_p2:format(cargo, amount, numjumps, traveldist, piracyrisk, (timelimit - time.get()):str()))
   misn.setReward(misn_reward:format(numstring(reward)))
end

-- Mission is accepted
function accept()
   if player.pilot():cargoFree() < amount then
      tk.msg(full[1], full[2]:format(amount, amount - player.pilot():cargoFree()))
      misn.finish()
   end
   player.pilot():cargoAdd( cargo, amount ) 
   local playerbest = cargoGetTransit( timelimit, numjumps, traveldist )
   player.pilot():cargoRm( cargo, amount ) 
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
      tk.msg(cargo_land_title, string.format( cargo.land_p2[rnd.rnd(1, #cargo_land_p2)], cargo_land_p1[rnd.rnd(1, #cargo_land_p1)], _(cargo) ))
   else
      -- Semi-random message for being late.
      tk.msg(cargo_land_title, string.format( cargo.land_p3[rnd.rnd(1, #cargo_land_p3)], cargo_land_p1[rnd.rnd(1, #cargo_land_p1)], _(cargo), reward/2, reward ))
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
