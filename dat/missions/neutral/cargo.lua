--[[
   -- These are regular cargo delivery missions. Pay is low, but so is difficulty.
   -- Most of these missions require BULK ships. Not for small ships!
--]]

require "dat/scripts/cargo_common.lua"
require "dat/scripts/numstring.lua"

misn_desc = _("%s in the %s system needs a delivery of %d tonnes of %s.")
misn_reward = _("%s credits")

cargosize = {}
cargosize[0] = _("Small") -- Note: indexed from 0, to match mission tiers.
cargosize[1] = _("Medium")
cargosize[2] = _("Sizeable")
cargosize[3] = _("Large")
cargosize[4] = _("Bulk")

title_p1 = {}
title_p1[1] = _(" cargo delivery to %s in the %s system")
title_p1[2] = _(" freight delivery to %s in the %s system")
title_p1[3] = _(" transport to %s in the %s system")
title_p1[4] = _(" delivery to %s in the %s system")

-- Note: please leave the trailing space on the line below! Needed to make the newline show up.
title_p2 = _([[ 
Cargo: %s (%d tonnes)
Jumps: %d
Travel distance: %d
Piracy Risk: %s]])

full = {}
full[1] = _("No room in ship")
full[2] = _("You don't have enough cargo space to accept this mission. You need %d tonnes of free space (you need %d more).")

piracyrisk = {}
piracyrisk[1] = _("None")
piracyrisk[2] = _("Low")
piracyrisk[3] = _("Medium")
piracyrisk[4] = _("High")
--=Landing=--

cargo_land_title = _("Delivery success!")

cargo_land_p1 = {}
cargo_land_p1[1] = _("The crates of ")
cargo_land_p1[2] = _("The drums of ")
cargo_land_p1[3] = _("The containers of ")

cargo_land_p2 = {}
cargo_land_p2[1] = _("%s%s are carried out of your ship by a sullen group of workers. The job takes inordinately long to complete, and the leader pays you without speaking a word.")
cargo_land_p2[2] = _("%s%s are rushed out of your vessel by a team shortly after you land. Before you can even collect your thoughts, one of them presses a credit chip in your hand and departs.")
cargo_land_p2[3] = _("%s%s are unloaded by an exhausted-looking bunch of dockworkers. Still, they make fairly good time, delivering your pay upon completion of the job.")
cargo_land_p2[4] = _("%s%s are unloaded by a team of robotic drones supervised by a human overseer, who hands you your pay when they finish.")

accept_title = _("Mission Accepted")

osd_title = _("Cargo mission")
osd_msg = _("Fly to %s in the %s system")

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
      tk.msg(cargo_land_title, cargo_land_p2[rnd.rnd(1, #cargo_land_p2)]:format( cargo_land_p1[rnd.rnd(1, #cargo_land_p1)], cargo ))
      player.pay(reward)
      misn.finish(true)
   end
end

function abort ()
   misn.finish(false)
end

