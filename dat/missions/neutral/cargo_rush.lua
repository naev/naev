--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Cargo Rush">
 <priority>5</priority>
 <chance>960</chance>
 <location>Computer</location>
 <cond>
  require("misn_test").cargo()
 </cond>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[
   These are rush cargo delivery missions. They can be failed! But, pay is higher to compensate.
   These missions require fast ships, but higher tiers may also require increased cargo space.
--]]
local pir = require 'common.pirate'
local fmt = require "format"
local vntk = require "vntk"
local car = require "common.cargo"
local lmisn = require "lmisn"


local misn_title = {}
-- Note: indexed from 0, to match mission tiers.
misn_title[0] = _("Courier transport to {pnt} in {sys} ({tonnes})")
misn_title[1] = _("Priority shipment to {pnt} in {sys} ({tonnes})")
misn_title[2] = _("Pressing cargo delivery to {pnt} in {sys} ({tonnes})")
misn_title[3] = _("Urgent cargo delivery to {pnt} in {sys} ({tonnes})")
misn_title[4] = _("Emergency cargo delivery to {pnt} in {sys} ({tonnes})")

local misn_desc = {}
-- Note: indexed from 0, to match mission tiers.
misn_desc[0] = _("Courier transport of {amount} of {cargo} to {pnt} in the {sys} system.")
misn_desc[1] = _("Priority shipment of {amount} of {cargo} to {pnt} in the {sys} system.")
misn_desc[2] = _("Pressing cargo delivery of {amount} of {cargo} to {pnt} in the {sys} system.")
misn_desc[3] = _("Urgent cargo delivery of {amount} of {cargo} to {pnt} in the {sys} system.")
misn_desc[4] = _("Emergency cargo delivery of {amount} of {cargo} to {pnt} in the {sys} system.")

local piracyrisk = {}
piracyrisk[1] = _("#nPiracy Risk:#0 None")
piracyrisk[2] = _("#nPiracy Risk:#0 Low")
piracyrisk[3] = _("#nPiracy Risk:#0 Medium")
piracyrisk[4] = _("#nPiracy Risk:#0 High")

--=Landing=--

local cargo_land = {
   _("The containers of {cargo} are carried out of your ship by a sullen group of workers. The job takes inordinately long to complete, and the leader pays you #g{credits}#0 without speaking a word."),
   _("Shortly after you land, a team rushes the containers of {cargo} out of your vessel. Before you can even collect your thoughts, one of the workers presses a credit chip worth #g{credits}#0 in your hand and departs."),
   _("The containers of {cargo} are unloaded by an exhausted-looking bunch of dockworkers. Still, they make fairly good time, delivering your pay of #g{credits}#0 upon completion of the job."),
   _("The containers of {cargo} are unloaded by a team of robotic drones supervised by a human overseer, who hands you your pay of #g{credits}#0 when they finish."),
}

local cargo_land_slow = {
   _("The containers of {cargo} are carried out of your ship by a sullen group of workers. They are not happy that they have to work overtime because you were late. You are paid only {credits} of the {reward} you were promised."),
   _("Shortly after you land, a team rushes the containers of {cargo} out of your vessel. Your late arrival is stretching quite a few schedules! Your pay is only {credits} instead of {reward} because of that."),
   _("The containers of {cargo} are unloaded by an exhausted-looking bunch of dockworkers. You missed the deadline, so your reward is only {credits} instead of the {reward} you were hoping for."),
}

-- Create the mission
function create()
   -- Note: this mission does not make any system claims.

   -- Calculate the route, distance, jumps, risk of piracy, and cargo to take
   mem.destplanet, mem.destsys, mem.numjumps, mem.traveldist, mem.cargo, mem.avgrisk, mem.tier = car.calculateRoute()
   if mem.destplanet == nil then
      misn.finish(false)
   end

   -- Calculate time limit. Depends on tier and distance.
   -- The second time limit is for the reduced reward.
   local stuperpx   = 0.2 - 0.025 * mem.tier
   local stuperjump = 10300 - 300 * mem.tier
   local stupertakeoff = 10300 - 75 * mem.tier
   local allowance  = mem.traveldist * stuperpx + mem.numjumps * stuperjump + stupertakeoff + 240 * mem.numjumps

   -- Allow extra time for refuelling stops.
   local jumpsperstop = 3 + math.min(mem.tier, 3)
   if mem.numjumps > jumpsperstop then
      allowance = allowance + math.floor((mem.numjumps-1) / jumpsperstop) * stuperjump
   end

   mem.timelimit  = time.get() + time.new(0, 0, allowance)
   mem.timelimit2 = time.get() + time.new(0, 0, allowance * 1.2)

   local riskreward
   if mem.avgrisk == 0 then
      piracyrisk = piracyrisk[1]
      riskreward = 0
   elseif mem.avgrisk <= 200 then
      piracyrisk = piracyrisk[2]
      riskreward = 10
   elseif mem.avgrisk <= 400 then
      piracyrisk = piracyrisk[3]
      riskreward = 25
   else
      piracyrisk = piracyrisk[4]
      riskreward = 50
   end

   -- Choose amount of cargo and mission reward. This depends on the mission tier.
   -- Note: Pay is independent from amount by design! Not all deals are equally attractive!
   mem.amount     = rnd.rnd(10 + 5 * mem.tier, 20 + 6 * mem.tier) -- 45 max (quicksilver)
   local jumpreward = commodity.price(mem.cargo)*1.8
   local distreward = math.log(300*commodity.price(mem.cargo))/80
   mem.reward     = 1.5^mem.tier * (mem.avgrisk*riskreward + mem.numjumps * jumpreward + mem.traveldist * distreward) * (1. + 0.05*rnd.twosigma())

   misn.setTitle( fmt.f( misn_title[mem.tier], {pnt=mem.destplanet, sys=mem.destsys, tonnes=fmt.tonnes_short(mem.amount)} ) )
   misn.markerAdd(mem.destplanet, "computer")
   car.setDesc( fmt.f( misn_desc[mem.tier], {cargo=_(mem.cargo), amount=fmt.tonnes(mem.amount), pnt=mem.destplanet, sys=mem.destsys} ), mem.cargo, mem.amount, mem.destplanet, mem.timelimit, piracyrisk )
   misn.setReward(mem.reward)
end

-- Mission is accepted
function accept()
   if player.pilot():cargoFree() < mem.amount then
      vntk.msg( _("No room in ship"), fmt.f(
         _("You don't have enough cargo space to accept this mission. It requires {tonnes_free} of free space ({tonnes_short} more than you have)."),
         { tonnes_free = fmt.tonnes(mem.amount), tonnes_short = fmt.tonnes( mem.amount - player.pilot():cargoFree() ) } ) )
      return
   end
   player.pilot():cargoAdd( mem.cargo, mem.amount )
   local playerbest = car.getTransit( mem.numjumps, mem.traveldist )
   player.pilot():cargoRm( mem.cargo, mem.amount )
   if mem.timelimit < playerbest then
      if not tk.yesno( _("Too slow"), fmt.f(
            _("This shipment must arrive within {time_limit}, but it will take at least {time} for your ship to reach {pnt}, missing the deadline. Accept the mission anyway?"),
	    {time_limit=(mem.timelimit - time.get()), time=(playerbest - time.get()), pnt=mem.destplanet} ) ) then
         return
      end
   elseif system.cur():jumpDist(mem.destsys, false, true) > mem.numjumps then
      if not tk.yesno( _("Unknown route"), fmt.f(
            _("The fastest route to {pnt} is not currently known to you. Landing to buy maps, spending time searching for unknown jumps, or taking a route longer than {jumps} may cause you to miss the deadline. Accept the mission anyway?"),
	    {pnt=mem.destplanet, jumps=fmt.jumps(mem.numjumps)} ) ) then
         return
      end
   end
   misn.accept()
   mem.intime = true
   misn.cargoAdd(mem.cargo, mem.amount)
   hook.land("land")
   hook.date(time.new(0, 0, 100), "tick") -- 100STU per tick
   tick() -- set OSD
end

-- Land hook
function land()
   if spob.cur() == mem.destplanet then
      if mem.intime then
      -- Semi-random message.
      lmisn.sfxMoney()
      vntk.msg( _("Successful Delivery"), fmt.f(cargo_land[rnd.rnd(1, #cargo_land)], {cargo=_(mem.cargo), credits=fmt.credits(mem.reward)}) )
   else
      -- Semi-random message for being late.
      lmisn.sfxMoney()
      vntk.msg( _("Successful Delivery"), fmt.f(cargo_land_slow[rnd.rnd(1, #cargo_land_slow)],
         {cargo=_(mem.cargo), credits=fmt.credits(mem.reward / 2), reward=fmt.credits(mem.reward)} ) )
      mem.reward = mem.reward / 2
   end
   player.pay(mem.reward)
   pir.reputationNormalMission(rnd.rnd(2,3))
   misn.finish(true)
   end
end

-- Date hook
function tick()
   local osd_msg = {}
   if mem.timelimit >= time.get() then
      -- Case still in time
      osd_msg[1] = fmt.f(_("Fly to {pnt} in the {sys} system before {time_limit}\n({time} remaining)"),
         {pnt=mem.destplanet, sys=mem.destsys, time_limit=mem.timelimit, time=(mem.timelimit - time.get())})
      misn.osdCreate(_("Rush cargo mission"), osd_msg)
   elseif mem.timelimit2 <= time.get() then
      -- Case missed second deadline
      player.msg( fmt.f(_("The delivery to {sys} has been canceled! You were too late."), {sys=mem.destsys}) )
      misn.finish(false)
   elseif mem.intime then
      -- Case missed first deadline
      osd_msg[1] = fmt.f(_("Fly to {pnt} in the {sys} system\n(deadline missed, but you can still make a late delivery if you hurry)"), {pnt=mem.destplanet, sys=mem.destsys})
      misn.osdCreate(_("Rush cargo mission"), osd_msg)
      mem.intime = false
   end
end
