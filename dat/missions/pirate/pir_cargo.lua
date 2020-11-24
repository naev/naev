--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Pirate Shipping">
  <avail>
   <priority>4</priority>
   <cond>faction.playerStanding("Pirate") &gt;= 0</cond>
   <chance>960</chance>
   <location>Computer</location>
   <faction>Pirate</faction>
  </avail>
  <notes>
   <tier>1</tier>
  </notes>
 </mission>
 --]]
--[[

   Handles the randomly generated Pirate cargo missions.

   Most of the code taken from Empire Shipping.

]]--

require "cargo_common.lua"
require "numstring.lua"


misn_desc = _("Pirate cargo transport of contraband goods to %s in the %s system.")

misn_details = _([[
Cargo: %s (%s)
Jumps: %d
Travel distance: %d
Time limit: %s]])

msg_timeup = _("MISSION FAILED: You have failed to deliver the goods on time!")

osd_title = _("Pirate Shipping")
osd_msg1 = _("Fly to %s in the %s system before %s\n(%s remaining)")

-- Use hidden jumps
cargo_use_hidden = true

-- Always available
cargo_always_available = true

--[[
--   Pirates shipping missions are always timed, but quite lax on the schedules
--   and pays a lot more then the rush missions
--]]


-- This is in cargo_common, but we need to increase the range
function cargo_selectMissionDistance ()
   return rnd.rnd( 3, 10 )
end


function create()
   -- Note: this mission does not make any system claims.

   origin_p, origin_s = planet.cur()
   local routesys = origin_s
   local routepos = origin_p:pos()

   -- target destination
   destplanet, destsys, numjumps, traveldist, cargo, avgrisk, tier = cargo_calculateRoute()
   if destplanet == nil or destplanet:faction() == faction.get("Pirate") then
      misn.finish(false)
   end

   -- We’re redefining the cargo
   local cargoes = {
      N_("Unmarked Boxes"),
      N_("Weapons"),
      N_("Drugs"),
      N_("Exotic Animals"),
      N_("Radioactive Materials"),
   }
   cargo = cargoes[rnd.rnd(1, #cargoes)]

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
   finished_mod = 2.0 -- Modifier that should tend towards 1.0 as Naev is finished as a game
   amount    = rnd.rnd(10 + 3 * tier, 20 + 4 * tier) 
   jumpreward = 1500
   distreward = 0.30
   reward    = 1.5^tier * (numjumps * jumpreward + traveldist * distreward) * finished_mod * (1. + 0.05*rnd.twosigma())
   
   misn.setTitle( string.format(
      _("PIRACY: Illegal Cargo transport (%s of %s)"), tonnestring(amount),
      _(cargo) ) )
   misn.markerAdd(destsys, "computer")
   misn.setDesc(
      misn_desc:format( destplanet:name(), destsys:name() ) .. "\n\n"
      .. misn_details:format(
         _(cargo), tonnestring(amount), numjumps, traveldist,
         (timelimit - time.get()):str() ) )
   misn.setReward( creditstring(reward) )

end

-- Mission is accepted
function accept()
   local playerbest = cargoGetTransit( timelimit, numjumps, traveldist )
   if timelimit < playerbest then
      if not tk.yesno( _("Too slow"), string.format(
            _("This shipment must arrive within %s, but it will take at least %s for your ship to reach %s, missing the deadline. Accept the mission anyway?"),
            (timelimit - time.get()):str(), (playerbest - time.get()):str(),
            destplanet:name() ) ) then
         misn.finish()
      end
   end
   if player.pilot():cargoFree() < amount then
      tk.msg( _("No room in ship"), string.format(
         _("You don't have enough cargo space to accept this mission. It requires %s of free space (%s more than you have)."),
         tonnestring(amount),
         tonnestring( amount - player.pilot():cargoFree() ) ) )
      misn.finish()
   end

   misn.accept()

   carg_id = misn.cargoAdd( cargo, amount )
   tk.msg( _("Mission Accepted"), string.format(
      _("%s of %s are loaded onto your ship."), tonnestring(amount),
      _(cargo) ) )
   local osd_msg = {}
   osd_msg[1] = osd_msg1:format(
      destplanet:name(), destsys:name(), timelimit:str(),
      ( timelimit - time.get() ):str() )
   misn.osdCreate(osd_title, osd_msg)
   hook.land( "land" ) -- only hook after accepting
   hook.date(time.create(0, 0, 100), "tick") -- 100STU per tick
end

-- Land hook
function land()
   if planet.cur() == destplanet then
         tk.msg( _("Successful Delivery"), string.format(
            _("The containers of %s are unloaded at the docks."), _(cargo) ) )
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
      local osd_msg = {}
      osd_msg[1] = osd_msg1:format(
         destplanet:name(), destsys:name(), timelimit:str(),
         ( timelimit - time.get() ):str() )
      misn.osdCreate(osd_title, osd_msg)
   elseif timelimit <= time.get() then
      -- Case missed deadline
      player.msg(msg_timeup)
      misn.finish(false)
   end
end
