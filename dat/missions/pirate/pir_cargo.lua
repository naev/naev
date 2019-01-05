--[[

   Handles the randomly generated Pirate cargo missions.

   Most of the code taken from Empire Shipping.

]]--

include "dat/scripts/cargo_common.lua"
include "dat/scripts/numstring.lua"

-- This is in cargo_common, but we need to increase the range…
function cargo_selectMissionDistance ()
   local seed = rnd.rnd()
   if     seed < 0.20 then missdist = 3
   elseif seed < 0.40 then missdist = 4
   elseif seed < 0.60 then missdist = 5
   elseif seed < 0.80 then missdist = 6
   else                    missdist = 7
   end

   return missdist
end

osd_title = _("Pirate Shipping")
osd_msg = {}
osd_msg[1] = _("Fly to %s in the %s system before %s.")
osd_msg[2] = _("You have %s remaining.")
osd_msg1 = _("Fly to %s in the %s system before %s.")
osd_msg2 = _("You have %s remaining.") -- Need to reuse.

misn_title = _("Pirate cargo transport (%d tonnes of %s)")
misn_desc = _("%d tons of %s needs to be shipped to %s in the %s system by %s (%s left).")
misn_reward = _("%s credits")
title = {}
title_p1 = _("Pirate: Cargo transport to %s in the %s system")
title_p2 = _([[ 
Cargo: %s (%d tonnes)
Jumps: %d
Travel distance: %d
Time limit: %s]])
full = {}
full[1] = _("Ship is full")
full[2] = _("Your ship is too full. You need to make room for %d more tons if you want to be able to accept the mission.")
slow = {}
slow[1] = _("Too slow")
slow[2] = _([[This shipment must arrive within %s, but it will take at least %s for your ship to reach %s.

Accept the mission anyway?]])
msg_title = {}
msg_title[1] = _("Mission Accepted")
msg_title[2] = _("Too many missions")
msg_title[3] = _("Successful Delivery")
msg_msg = {}
msg_msg[1] = _("%d tons of %s are loaded onto your ship.")
msg_msg[2] = _("You have too many active missions.")
msg_msg[3] = _("The %s is unloaded at the docks.")
msg_msg[4] = _("The %s is passed through the airlock.")
miss = {}
miss[1]= _("Cargo Missing")
miss[2] = _("You are missing the %d tons of %s!.")
miss[3] = _("MISSION FAILED: You have failed to deliver the goods on time!")

--[[
--   Pirates shipping missions are always timed, but quite lax on the schedules
--   and pays a lot more then the rush missions
--]]

-- Create the mission
function create()
   -- Note: this mission does not make any system claims.

   origin_p, origin_s = planet.cur()
   local routesys = origin_s
   local routepos = origin_p:pos()

   -- target destination
   destplanet, destsys, numjumps, traveldist, cargo, _, tier = cargo_calculateRoute()
   if destplanet == nil then
      misn.finish(false)
   end

   -- We’re redefining the cargo
   local cargoes = {
      "Unmarked Boxes",
      "Weapons",
      "Drugs",
      "Exotic Animals",
      "Radioactive Materials",
   }
   cargo = cargoes[rnd.rnd(1, #cargoes)]

   -- It is my opinion that those cargos should come from Pirate worlds and be
   -- destined to any other worlds, for black markets and such. Besides, 
   -- Pirate worlds are too far away from one another.
   --if destplanet:faction() ~= faction.get( "Pirate" ) then
   --   misn.finish(false)
   --end

   -- mission generics
   stuperpx   = 0.3 - 0.015 * tier
   stuperjump = 11000 - 75 * tier
   stupertakeoff = 15000
   timelimit  = time.get() + time.create(0, 0, traveldist * stuperpx + numjumps * stuperjump + stupertakeoff + 480 * numjumps)

   -- Allow extra time for refuelling stops.
   local jumpsperstop = 3 + math.min(tier, 1)
   if numjumps > jumpsperstop then
      timelimit:add(time.create( 0, 0, math.floor((numjumps-1) / jumpsperstop) * stuperjump ))
   end
   
   -- Choose amount of cargo and mission reward. This depends on the mission tier.
   finished_mod = 2.0 -- Modifier that should tend towards 1.0 as naev is finished as a game
   amount    = rnd.rnd(10 + 3 * tier, 20 + 4 * tier) 
   jumpreward = 1500
   distreward = 0.30
   reward    = 1.5^tier * (numjumps * jumpreward + traveldist * distreward) * finished_mod * (1. + 0.05*rnd.twosigma())
   
   misn.setTitle( misn_title:format( amount, cargo ) )
   misn.markerAdd(destsys, "computer")
   misn.setDesc(title_p1:format(destplanet:name(), destsys:name()) .. title_p2:format(cargo, amount, numjumps, traveldist, (timelimit - time.get()):str()))
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
      n = var.peek("ps_misn")
      if n ~= nil then
         var.push("ps_misn", n+1)
      else
         var.push("ps_misn", 1)
      end

      -- increase faction
      faction.modPlayerSingle("Pirate", rnd.rnd(2, 4))
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
