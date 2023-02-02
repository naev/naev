--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Cargo">
 <priority>6</priority>
 <chance>960</chance>
 <location>Computer</location>
 <cond>
   local f = spob.cur():faction()
   if f then
      local ft = f:tags()
      if ft.generic or ft.misn_cargo then
         return true
      end
   end
   return false
 </cond>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[
   These are regular cargo delivery missions. Pay is low, but so is difficulty.
   Most of these missions require BULK ships. Not for small ships!
--]]
local pir = require "common.pirate"
local fmt = require "format"
local vntk = require "vntk"
local car = require "common.cargo"
local lmisn = require "lmisn"


local misn_desc = {}
-- Note: indexed from 0 to match mission tiers.
misn_desc[0] = _("Small shipment of {amount} of {cargo} to {pnt} in the {sys} system.")
misn_desc[1] = _("Medium shipment of {amount} of {cargo} to {pnt} in the {sys} system.")
misn_desc[2] = _("Sizable cargo delivery of {amount} of {cargo} to {pnt} in the {sys} system.")
misn_desc[3] = _("Large cargo delivery of {amount} of {cargo} to {pnt} in the {sys} system.")
misn_desc[4] = _("Bulk freight delivery of {amount} of {cargo} to {pnt} in the {sys} system.")

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

-- Create the mission
function create()
   -- Note: this mission does not make any system claims.

   -- Calculate the route, distance, jumps, risk of piracy, and cargo to take
   mem.destplanet, mem.destsys, mem.numjumps, mem.traveldist, mem.cargo, mem.avgrisk, mem.tier = car.calculateRoute()
   if mem.destplanet == nil then
      misn.finish(false)
   end

   local riskreward
   if mem.avgrisk == 0 then
      piracyrisk = piracyrisk[1]
      riskreward = 0
   elseif mem.avgrisk <= 150 then
      piracyrisk = piracyrisk[2]
      riskreward = 10
   elseif mem.avgrisk <= 300 then
      piracyrisk = piracyrisk[3]
      riskreward = 25
   else
      piracyrisk = piracyrisk[4]
      riskreward = 50
   end

   -- Choose amount of cargo and mission reward. This depends on the mission tier.
   -- Reward depends on type of cargo hauled. Hauling expensive commodities gives a better deal.
   -- Note: Pay is independent from amount by design! Not all deals are equally attractive!
   mem.amount = rnd.rnd(5 + 25 * mem.tier, 20 + 60 * mem.tier)
   local jumpreward = commodity.price(mem.cargo)*1.5
   local distreward = math.log(100*commodity.price(mem.cargo))/80
   mem.reward = 1.5^mem.tier * (mem.avgrisk*riskreward + mem.numjumps * jumpreward + mem.traveldist * distreward) * (1 + 0.05*rnd.twosigma())

   misn.setTitle( fmt.f(_("Shipment to {pnt} in {sys} ({tonnes})"),
         {pnt=mem.destplanet, sys=mem.destsys, tonnes=fmt.tonnes_short(mem.amount)} ) )
   misn.markerAdd(mem.destplanet, "computer")
   car.setDesc( fmt.f( misn_desc[mem.tier], {cargo=_(mem.cargo), amount=fmt.tonnes(mem.amount), pnt=mem.destplanet, sys=mem.destsys} ), mem.cargo, mem.amount, mem.destplanet, nil, piracyrisk )
   misn.setReward( fmt.credits(mem.reward) )
end

-- Mission is accepted
function accept ()
   local freecargo = player.pilot():cargoFree()
   if freecargo < mem.amount then
      vntk.msg( _("No room in ship"), fmt.f(
         _("You don't have enough cargo space to accept this mission. It requires {tonnes_free} of free space ({tonnes_short} more than you have)."),
         { tonnes_free = fmt.tonnes(mem.amount), tonnes_short = fmt.tonnes( mem.amount - freecargo ) } ) )
      return
   end
   misn.accept()
   misn.cargoAdd(mem.cargo, mem.amount)
   misn.osdCreate(_("Cargo mission"), {fmt.f(_("Fly to {pnt} in the {sys} system"), {pnt=mem.destplanet, sys=mem.destsys})})
   hook.land("land")
end

-- Land hook
function land()
   if spob.cur() ~= mem.destplanet then
      return
   end

   -- Semi-random message.
   lmisn.sfxMoney()
   vntk.msg( _("Delivery success!"), fmt.f(cargo_land[rnd.rnd(1, #cargo_land)], {cargo=_(mem.cargo), credits=fmt.credits(mem.reward)}) )
   player.pay(mem.reward)
   pir.reputationNormalMission(rnd.rnd(2,3))
   misn.finish(true)
end
