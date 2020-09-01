--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Cargo Rush">
  <avail>
   <priority>5</priority>
   <chance>960</chance>
   <location>Computer</location>
   <faction>Dvaered</faction>
   <faction>Empire</faction>
   <faction>Frontier</faction>
   <faction>Goddard</faction>
   <faction>Independent</faction>
   <faction>Proteron</faction>
   <faction>Sirius</faction>
   <faction>Soromid</faction>
   <faction>Thurion</faction>
   <faction>Traders Guild</faction>
   <faction>Za'lek</faction>
  </avail>
  <notes>
   <tier>1</tier>
  </notes>
 </mission>
 --]]
--[[
   -- These are rush cargo delivery missions. They can be failed! But, pay is higher to compensate.
   -- These missions require fast ships, but higher tiers may also require increased cargo space.
--]]

require "cargo_common.lua"
require "numstring.lua"


misn_title = {}
-- Note: indexed from 0, to match mission tiers.
misn_title[0] = _("Courier transport to %s in %s (%s)")
misn_title[1] = _("Priority shipment to %s in %s (%s)")
misn_title[2] = _("Pressing cargo delivery to %s in %s (%s)")
misn_title[3] = _("Urgent cargo delivery to %s in %s (%s)")
misn_title[4] = _("Emergency cargo delivery to %s in %s (%s)")

misn_desc = {}
-- Note: indexed from 0, to match mission tiers.
misn_desc[0] = _("Courier transport to %s in the %s system.")
misn_desc[1] = _("Priority shipment to %s in the %s system.")
misn_desc[2] = _("Pressing cargo delivery to %s in the %s system.")
misn_desc[3] = _("Urgent cargo delivery to %s in the %s system.")
misn_desc[4] = _("Emergency cargo delivery to %s in the %s system.")

misn_details = _([[
Cargo: %s (%s)
Jumps: %d
Travel distance: %d
%s
Time limit: %s
]])

piracyrisk = {}
piracyrisk[1] = _("Piracy Risk: None")
piracyrisk[2] = _("Piracy Risk: Low")
piracyrisk[3] = _("Piracy Risk: Medium")
piracyrisk[4] = _("Piracy Risk: High")

--=Landing=--

cargo_land_title = _("Successful Delivery")

cargo_land = {}
cargo_land[1] = _("The containers of %s are carried out of your ship by a sullen group of workers. The job takes inordinately long to complete, and the leader pays you without speaking a word.")
cargo_land[2] = _("The containers of %s are rushed out of your vessel by a team shortly after you land. Before you can even collect your thoughts, one of them presses a credit chip in your hand and departs.")
cargo_land[3] = _("The containers of %s are unloaded by an exhausted-looking bunch of dockworkers. Still, they make fairly good time, delivering your pay upon completion of the job.")

cargo_land_slow = {}
cargo_land_slow[1] = _("The containers of %s are carried out of your ship by a sullen group of workers. They are not happy that they have to work overtime because you were late. You are paid only %s of the %s you were promised.")
cargo_land_slow[2] = _("The containers of %s are rushed out of your vessel by a team shortly after you land. Your late arrival is stretching quite a few schedules! Your pay is only %s instead of %s because of that.")
cargo_land_slow[3] = _("The containers of %s are unloaded by an exhausted-looking bunch of dockworkers. You missed the deadline, so your reward is only %s instead of the %s you were hoping for.")

msg_timeup = _("The delivery to %s has been canceled! You were too late.")

osd_title = _("Rush cargo mission")
osd_msg1 = _("Fly to %s in the %s system before %s\n(%s remaining)")
osd_timeup = _("Fly to %s in the %s system\n(deadline missed, but you can still make a late delivery if you hurry)")

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
   
   misn.setTitle( misn_title[tier]:format(
      destplanet:name(), destsys:name(), tonnestring(amount) ) )
   misn.markerAdd(destsys, "computer")
   misn.setDesc(
      misn_desc[tier]:format( destplanet:name(), destsys:name() ) .. "\n\n"
      .. misn_details:format(
         cargo, tonnestring(amount), numjumps, traveldist, piracyrisk,
         (timelimit - time.get()):str() ) )
   misn.setReward( creditstring(reward) )
end

-- Mission is accepted
function accept()
   if player.pilot():cargoFree() < amount then
      tk.msg( _("No room in ship"), string.format(
         _("You don't have enough cargo space to accept this mission. It requires %s of free space (%s more than you have)."),
         tonnestring(amount),
         tonnestring( amount - player.pilot():cargoFree() ) ) )
      misn.finish()
   end
   player.pilot():cargoAdd( cargo, amount ) 
   local playerbest = cargoGetTransit( timelimit, numjumps, traveldist )
   player.pilot():cargoRm( cargo, amount ) 
   if timelimit < playerbest then
      if not tk.yesno( _("Too slow"), string.format(
            _("This shipment must arrive within %s, but it will take at least %s for your ship to reach %s, missing the deadline. Accept the mission anyway?"),
            (timelimit - time.get()):str(), (playerbest - time.get()):str(),
            destplanet:name() ) ) then
         misn.finish()
      end
   end
   misn.accept()
   intime = true
   misn.cargoAdd(cargo, amount) -- TODO: change to jettisonable cargo once custom commodities are in. For piracy purposes.
   local osd_msg = {}
   osd_msg[1] = osd_msg1:format(
      destplanet:name(), destsys:name(), timelimit:str(),
      (timelimit - time.get()):str() )
   misn.osdCreate(osd_title, osd_msg)
   hook.land("land")
   hook.date(time.create(0, 0, 100), "tick") -- 100STU per tick
end

-- Land hook
function land()
   if planet.cur() == destplanet then
      if intime then
      -- Semi-random message.
      tk.msg( cargo_land_title, cargo_land[rnd.rnd(1, #cargo_land)]:format(cargo) )
   else
      -- Semi-random message for being late.
      tk.msg( cargo_land_title, cargo_land_slow[rnd.rnd(1, #cargo_land_slow)]:format(
         cargo, creditstring(reward / 2), creditstring(reward) ) )
      reward = reward / 2
   end
   player.pay(reward)
   misn.finish(true)
   end
end

-- Date hook
function tick()
   local osd_msg = {}
   if timelimit >= time.get() then
      -- Case still in time
      osd_msg[1] = osd_msg1:format(
         destplanet:name(), destsys:name(), timelimit:str(),
         (timelimit - time.get()):str() )
      misn.osdCreate(osd_title, osd_msg)
   elseif timelimit2 <= time.get() then
      -- Case missed second deadline
      player.msg( msg_timeup:format( destsys:name() ) )
      misn.finish(false)
   elseif intime then
      -- Case missed first deadline
      osd_msg[1] = osd_timeup:format( destplanet:name(), destsys:name() )
      misn.osdCreate(osd_title, osd_msg)
      intime = false
   end
end
