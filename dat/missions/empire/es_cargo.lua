--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Empire Shipping">
 <priority>3</priority>
 <cond>
   if system.cur():reputation("Empire") &lt; 0 or faction.reputationGlobal("Empire") &lt; 0 then
      return false
   end
   if var.peek("es_cargo") ~= true then
      return false
   end
   return require("misn_test").computer()
 </cond>
 <chance>350</chance>
 <done>Empire Recruitment</done>
 <location>Computer</location>
 <faction>Empire</faction>
</mission>
--]]
--[[

   Handles the randomly generated Empire mem.cargo missions.

]]--
local pir = require "common.pirate"
local car = require "common.cargo"
local fmt = require "format"
local lmisn = require "lmisn"
local emp = require "common.empire"
local vntk = require "vntk"

local piracyrisk = {}
piracyrisk[1] = _("#nPiracy Risk:#0 None")
piracyrisk[2] = _("#nPiracy Risk:#0 Low")
piracyrisk[3] = _("#nPiracy Risk:#0 Medium")
piracyrisk[4] = _("#nPiracy Risk:#0 High")

--[[
--    Empire shipping missions are always timed, but quite lax on the schedules
--    pays a bit more then the rush missions
--]]

-- Create the mission
function create()
   -- Note: this mission does not make any system claims.

   mem.origin_p, mem.origin_s = spob.cur()

   -- target destination
   mem.destplanet, mem.destsys, mem.numjumps, mem.traveldist, mem.cargo, mem.avgrisk, mem.tier = car.calculateRoute()
   if mem.destplanet == nil then
      misn.finish(false)
   end
   if mem.destplanet:faction() ~= faction.get( "Empire" ) then
      misn.finish(false)
   end

   -- mission generics
   local stuperpx   = 0.3 - 0.015 * mem.tier
   local stuperjump = 11000 - 75 * mem.tier
   local stupertakeoff = 15000
   mem.timelimit  = time.get() + time.new(0, 0, mem.traveldist * stuperpx + mem.numjumps * stuperjump + stupertakeoff + 480 * mem.numjumps)

   -- Allow extra time for refuelling stops.
   local jumpsperstop = 3 + math.min(mem.tier, 2)
   if mem.numjumps > jumpsperstop then
      mem.timelimit:add(time.new( 0, 0, math.floor((mem.numjumps-1) / jumpsperstop) * stuperjump ))
   end

   --Determine risk of piracy
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
   mem.amount     = rnd.rnd(10 + 3 * mem.tier, 20 + 4 * mem.tier)
   local jumpreward = commodity.price(mem.cargo)*2.5
   local distreward = math.log(300*commodity.price(mem.cargo))/80
   mem.reward     = 1.5^mem.tier * (mem.avgrisk*riskreward + mem.numjumps * jumpreward + mem.traveldist * distreward) * (1. + 0.05*rnd.twosigma())

   misn.setTitle( fmt.f( emp.prefix.._("Cargo transport to {pnt} in {sys} ({tonnes})"),
         {pnt=mem.destplanet, sys=mem.destsys, tonnes=fmt.tonnes(mem.amount)} ) )
   misn.markerAdd(mem.destplanet, "computer")
   car.setDesc( fmt.f(_("Official Empire cargo transport to {pnt} in the {sys} system."), {pnt=mem.destplanet, sys=mem.destsys} ), mem.cargo, mem.amount, mem.destplanet, mem.timelimit, piracyrisk )
   misn.setReward(mem.reward)
end

-- Mission is accepted
function accept()
   local playerbest = car.getTransit( mem.numjumps, mem.traveldist )
   if mem.timelimit < playerbest then
      if not vntk.yesno( _("Too slow"), fmt.f(
            _("This shipment must arrive within {time_limit}, but it will take at least {time} for your ship to reach {pnt}, missing the deadline. Accept the mission anyway?"),
         { time_limit=(mem.timelimit - time.get()), time=(playerbest - time.get()), pnt=mem.destplanet} ) ) then
         return
      end
   end
   local fs = player.fleetCargoMissionFree()
   if fs < mem.amount then
      vntk.msg( _("No room in ship"), fmt.f(
         _("You don't have enough cargo space to accept this mission. It requires {tonnes_free} of free space ({tonnes_short} more than you have)."),
         { tonnes_free = fmt.tonnes(mem.amount), tonnes_short = fmt.tonnes( mem.amount - fs ) } ) )
      return
   end

   misn.accept()

   mem.carg_id = misn.cargoAdd( mem.cargo, mem.amount )
   vntk.msg( _("Mission Accepted"), fmt.f(
      _("The Empire workers load the {tonnes} of {cargo} onto your ship."),
      {tonnes=fmt.tonnes(mem.amount), cargo=_(mem.cargo)} ) )
   hook.land( "land" ) -- only hook after accepting
   hook.date(time.new(0, 0, 100), "tick") -- 100STU per tick
   tick() -- set OSD
end

-- Land hook
function land()
   if spob.cur() == mem.destplanet then
      vntk.msg( _("Successful Delivery"), fmt.f(
         _("The Empire workers unload the {cargo} at the docks."), {cargo=_(mem.cargo)} ) )
      player.pay(mem.reward)
      local n = var.peek("es_misn")
      if n ~= nil then
         var.push("es_misn", n+1)
      else
         var.push("es_misn", 1)
      end

      -- increase faction
      local reputation = rnd.rnd(1, 2)
      faction.hit( "Empire", reputation )
      pir.reputationNormalMission(reputation)
      misn.finish(true)
   end
end

-- Date hook
function tick()
   if mem.timelimit >= time.get() then
      -- Case still in time
      local osd_msg = {}
      osd_msg[1] = fmt.f(_("Fly to {pnt} in the {sys} system before {time_limit}\n({time} remaining)"),
         {pnt=mem.destplanet, sys=mem.destsys, time_limit=mem.timelimit, time=(mem.timelimit - time.get())})
      misn.osdCreate(_("Empire Shipping"), osd_msg)
   elseif mem.timelimit <= time.get() then
      -- Case missed deadline
      lmisn.fail(_("You have failed to deliver the goods to the Empire on time!"))
   end
end
