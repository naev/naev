--[[
   
   Handles the randomly generated Empire long-distance cargo missions.
   
]]--

require "dat/scripts/cargo_common.lua"
require "dat/scripts/numstring.lua"

misn_title = "ES: Long distance cargo transport (%d tonnes of %s)"
misn_desc = _("The Empire needs to ship %d tonnes of %s to %s in the %s system by %s (%s left).")
misn_reward = _("%s credits")

title = _([[ES: Long distance cargo transport to %s in the %s system
   Cargo: %s (%d tonnes)
   Jumps: %d
   Travel distance: %d
   Piracy risk: %s
   Time limit: %s]])
   
   
full = {}
full[1] = _("Ship is full")
full[2] = _("Your ship is too full. You need to make room for %d more tonnes if you want to be able to accept the mission.")

slow = {}
slow[1] = _("Too slow")
slow[2] = _([[This shipment must arrive within %s, but it will take at least %s for your ship to reach %s, and the Empire is not fond of delays. Accept the mission anyway?]])

piracyrisk = {}
piracyrisk[1] = _("None")
piracyrisk[2] = _("Low")
piracyrisk[3] = _("Medium")
piracyrisk[4] = _("High")

msg_title = {}
msg_title[1] = _("Mission Accepted")
msg_title[2] = _("Too many missions")
msg_title[3] = _("Successful Delivery")
msg_msg = {}
msg_msg[1] = _("The Empire workers load the %d tonnes of %s onto your ship.")
msg_msg[2] = _("You have too many active missions.")
msg_msg[3] = _("The Empire workers unload the %s at the docks.")
miss = {}
miss[1]= _("Cargo Missing")
miss[2] = _("You are missing the %d tonnes of %s!.")
miss[3] = _("MISSION FAILED: You have failed to deliver the goods to the Empire on time!")

osd_title = _("Long Distance Empire Shipping")
osd_msg = {}
osd_msg[1] = _("Fly to %s in the %s system before %s")
osd_msg[2] = _("You have %s remaining")
osd_msg1 = _("Fly to %s in the %s system before %s")
osd_msg2 = _("You have %s remaining") -- Need to reuse.

--[[
   -- Empire shipping missions are always timed, but quite lax on the schedules
   -- pays a bit more then the rush missions
--]]

-- Create the mission
function create()
   -- Note: this mission does not make any system claims.
   
   origin_p, origin_s = planet.cur()
   local routesys = origin_s
   local routepos = origin_p:pos()
   
   -- target destination
   destplanet, destsys, numjumps, traveldist, cargo, avgrisk, tier = cargo_calculateRoute()
   if destplanet == nil then
      misn.finish(false)
   end
   if destplanet:faction() == faction.get( "Empire" ) then
      misn.finish(false)
   elseif destplanet:faction() == faction.get( "Pirate" ) then
      misn.finish(false)
   elseif destplanet:faction() == faction.get( "Proteron" ) then
      misn.finish(false)        
   elseif destplanet:faction() == faction.get( "Independent" ) then
      misn.finish(false)        
   elseif numjumps < 3 then
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
   finished_mod = 2.0 -- Modifier that should tend towards 1.0 as naev is finished as a game
   amount     = rnd.rnd(10 + 3 * tier, 20 + 4 * tier) 
	jumpreward = commodity.price(cargo)*1.5
   distreward = math.log(300*commodity.price(cargo))/100
   reward     = 1.5^tier * (avgrisk*riskreward + numjumps * jumpreward + traveldist * distreward) * finished_mod * (1. + 0.05*rnd.twosigma())
   
   misn.setTitle( misn_title:format( amount, cargo ) )
   misn.markerAdd(destsys, "computer")
   misn.setDesc(title:format(destplanet:name(), destsys:name(), cargo, amount, numjumps, traveldist, piracyrisk, (timelimit - time.get()):str()))
   misn.setReward(misn_reward:format(numstring(reward)))
   
end

-- Mission is accepted
function accept()
   local playerbest = cargoGetTransit( timelimit, numjumps, traveldist )
   if timelimit < playerbest then
      if not tk.yesno( slow[1], slow[2]:format( (timelimit - time.get()):str(), (playerbest - time.get()):str(), destplanet:name()) ) then
         misn.finish()
      end
   end
   if player.pilot():cargoFree() < amount then
      tk.msg( full[1], string.format( full[2], amount-player.pilot():cargoFree() ))
      misn.finish()
   end
   
   if misn.accept() then -- able to accept the mission, hooks BREAK after accepting
      carg_id = misn.cargoAdd( cargo, amount )
      tk.msg( msg_title[1], string.format( msg_msg[1], amount, cargo ))
      osd_msg[1] = osd_msg1:format(destplanet:name(), destsys:name(), timelimit:str())
      osd_msg[2] = osd_msg2:format((timelimit - time.get()):str())
      misn.osdCreate(osd_title, osd_msg)
      hook.land( "land" ) -- only hook after accepting
      hook.date(time.create(0, 0, 100), "tick") -- 100STU per tick
   else
      tk.msg( msg_title[2], msg_msg [2] )
      misn.finish()
   end
end

-- Land hook
function land()
   if planet.cur() == destplanet then
      tk.msg( msg_title[3], string.format( msg_msg[3], cargo ))
      player.pay(reward)
      n = var.peek("es_misn")
      if n ~= nil then
         var.push("es_misn", n+1)
         else
         var.push("es_misn", 1)
      end
      
      -- increase faction
      faction.modPlayerSingle("Empire", rnd.rnd(4, 6))
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
      elseif timelimit <= time.get() then
      -- Case missed deadline
      player.msg(miss[3])
      abort()
   end
end

function abort ()
   misn.finish(false)
end
