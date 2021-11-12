--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Empire Shipping">
  <avail>
   <priority>3</priority>
   <cond>faction.playerStanding("Empire") &gt;= 0 and var.peek("es_cargo") == true</cond>
   <chance>350</chance>
   <done>Empire Recruitment</done>
   <location>Computer</location>
   <faction>Empire</faction>
  </avail>
 </mission>
 --]]
--[[

   Handles the randomly generated Empire cargo missions.

]]--
local pir = require "common.pirate"
local car = require "common.cargo"
local fmt = require "format"

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

   origin_p, origin_s = planet.cur()

   -- target destination
   destplanet, destsys, numjumps, traveldist, cargo, avgrisk, tier = car.calculateRoute()
   if destplanet == nil then
      misn.finish(false)
   end
   if destplanet:faction() ~= faction.get( "Empire" ) then
      misn.finish(false)
   end

   -- mission generics
   stuperpx   = 0.3 - 0.015 * tier
   stuperjump = 11000 - 75 * tier
   stupertakeoff = 15000
   timelimit  = time.get() + time.create(0, 0, traveldist * stuperpx + numjumps * stuperjump + stupertakeoff + 480 * numjumps)

   -- Allow extra time for refuelling stops.
   local jumpsperstop = 3 + math.min(tier, 2)
   if numjumps > jumpsperstop then
      timelimit:add(time.create( 0, 0, math.floor((numjumps-1) / jumpsperstop) * stuperjump ))
   end

   --Determine risk of piracy
   local riskreward
    if avgrisk == 0 then
      piracyrisk = piracyrisk[1]
      riskreward = 0
   elseif avgrisk <= 200 then
      piracyrisk = piracyrisk[2]
      riskreward = 10
   elseif avgrisk <= 400 then
      piracyrisk = piracyrisk[3]
      riskreward = 25
   else
      piracyrisk = piracyrisk[4]
      riskreward = 50
   end

   -- Choose amount of cargo and mission reward. This depends on the mission tier.
   amount     = rnd.rnd(10 + 3 * tier, 20 + 4 * tier)
   local jumpreward = commodity.price(cargo)*2.5
   local distreward = math.log(300*commodity.price(cargo))/80
   reward     = 1.5^tier * (avgrisk*riskreward + numjumps * jumpreward + traveldist * distreward) * (1. + 0.05*rnd.twosigma())

   misn.setTitle( fmt.f(
      _("ES: Cargo transport to {pnt} in {sys} ({tonnes})"), {pnt=destplanet, sys=destsys, tonnes=fmt.tonnes(amount)} ) )
   misn.markerAdd(destsys, "computer")
   car.setDesc( fmt.f(_("Official Empire cargo transport to {pnt} in the {sys} system."), {pnt=destplanet, sys=destsys} ), cargo, amount, destplanet, timelimit, piracyrisk )
   misn.setReward( fmt.credits(reward) )
end

-- Mission is accepted
function accept()
   local playerbest = car.getTransit( numjumps, traveldist )
   if timelimit < playerbest then
      if not tk.yesno( _("Too slow"), fmt.f(
            _("This shipment must arrive within {time_limit}, but it will take at least {time} for your ship to reach {pnt}, missing the deadline. Accept the mission anyway?"),
	    {time_limit=(timelimit - time.get()), time=(playerbest - time.get()), pnt=destplanet} ) ) then
         misn.finish()
      end
   end
   if player.pilot():cargoFree() < amount then
      tk.msg( _("No room in ship"), fmt.f(
         _("You don't have enough cargo space to accept this mission. It requires {tonnes_free} of free space ({tonnes_short}more than you have)."),
         { tonnes_free = fmt.tonnes(amount), tonnes_short = fmt.tonnes( amount - player.pilot():cargoFree() ) } ) )
      misn.finish()
   end

   misn.accept()

   carg_id = misn.cargoAdd( cargo, amount )
   tk.msg( _("Mission Accepted"), fmt.f(
      _("The Empire workers load the {tonnes} of {cargo} onto your ship."),
      {tonnes=fmt.tonnes(amount), cargo=_(cargo)} ) )
   hook.land( "land" ) -- only hook after accepting
   hook.date(time.create(0, 0, 100), "tick") -- 100STU per tick
   tick() -- set OSD
end

-- Land hook
function land()
   if planet.cur() == destplanet then
      tk.msg( _("Successful Delivery"), fmt.f(
         _("The Empire workers unload the {cargo} at the docks."), {cargo=_(cargo)} ) )
      player.pay(reward)
      n = var.peek("es_misn")
      if n ~= nil then
         var.push("es_misn", n+1)
      else
         var.push("es_misn", 1)
      end

      -- increase faction
      local reputation = rnd.rnd(2, 4)
      faction.modPlayerSingle("Empire", reputation)
      pir.reputationNormalMission(reputation)
      misn.finish(true)
   end
end

-- Date hook
function tick()
   if timelimit >= time.get() then
      -- Case still in time
      local osd_msg = {}
      osd_msg[1] = fmt.f(_("Fly to {pnt} in the {sys} system before {time_limit}\n({time} remaining)"),
         {pnt=destplanet, sys=destsys, time_limit=timelimit, time=(timelimit - time.get())})
      misn.osdCreate(_("Empire Shipping"), osd_msg)
   elseif timelimit <= time.get() then
      -- Case missed deadline
      player.msg(_("#rMISSION FAILED: You have failed to deliver the goods to the Empire on time!#0"))
      misn.finish(false)
   end
end
