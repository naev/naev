--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Empire Shipping 2">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>2</priority>
  <cond>faction.playerStanding("Empire") &gt;= 0 and faction.playerStanding("Dvaered") &gt;= 0 and faction.playerStanding("FLF") &lt; 10</cond>
  <chance>50</chance>
  <done>Empire Shipping 1</done>
  <location>Bar</location>
  <planet>Halir</planet>
 </avail>
 <notes>
  <campaign>Empire Shipping</campaign>
 </notes>
</mission>
--]]
--[[

   Empire Shipping Dangerous Cargo Delivery

   Author: bobbens
      minor edits by Infiltrator

]]--
local fmt = require "format"
local emp = require "common.empire"

-- Mission details
-- Errors

function create ()
   -- Note: this mission does not make any system claims.

   -- Planet targets
   mem.pickup,mem.pickupsys  = planet.getLandable( "Selphod" )
   mem.dest,mem.destsys      = planet.getLandable( "Cerberus" )
   mem.ret,mem.retsys        = planet.getLandable( "Halir" )
   if mem.pickup==nil or mem.dest==nil or mem.ret==nil then
      misn.finish(false)
   end

   -- Bar NPC
   misn.setNPC( _("Soldner"), "empire/unique/soldner.webp", _("You see Commander Soldner who is expecting you.") )
end

function accept ()

   -- See if accept mission
   if not tk.yesno( _("Commander Soldner"), _([[You approach Commander Soldner, who seems to be waiting for you.
"Hello, ready for your next mission?"]]) ) then
      misn.finish()
   end

   misn.accept()

   -- target destination
   mem.misn_marker       = misn.markerAdd( mem.pickupsys, "low" )

   -- Mission details
   mem.misn_stage = 0
   misn.setTitle(_("Empire Shipping Delivery"))
   misn.setReward( fmt.credits( emp.rewards.es01 ) )
   misn.setDesc( fmt.f(_("Pick up a package at {pnt} in the {sys} system"), {pnt=mem.pickup, sys=mem.pickupsys}) )

   -- Flavour text and mini-briefing
   tk.msg( _("Commander Soldner"), fmt.f( _([[Commander Soldner begins, "We have an important package that we must take from {pickup_pnt} in the {pickup_sys} system to {dropoff_pnt} in the {dropoff_sys} system. We have reason to believe that it is also wanted by external forces.
    "The plan is to send an advance convoy with guards to make the run in an attempt to confuse possible enemies. You will then go in and do the actual delivery by yourself. This way we shouldn't arouse suspicion. You are to report here when you finish delivery and you'll be paid {credits}."]]), {pickup_pnt=mem.pickup, pickup_sys=mem.pickupsys, dropoff_pnt=mem.dest, dropoff_sys=mem.destsys, credits=fmt.credits(emp.rewards.es01)} ) )
   misn.osdCreate(_("Empire Shipping Delivery"), {
      fmt.f(_("Pick up a package at {pnt} in the {sys} system"), {pnt=mem.pickup, sys=mem.pickupsys}),
   })

   -- Set up the goal
   tk.msg( _("Commander Soldner"), _([["Avoid hostility at all costs. The package must arrive at its destination. Since you are undercover, Empire ships won't assist you if you come under fire, so stay sharp. Good luck."]]) )

   -- Set hooks
   hook.land("land")
   hook.enter("enter")
end


function land ()
   mem.landed = planet.cur()

   if mem.landed == mem.pickup and mem.misn_stage == 0 then

      -- Make sure player has room.
      if player.pilot():cargoFree() < 3 then
         local needed = 3 - player.pilot():cargoFree()
         tk.msg( _("Need More Space"), string.format( n_(
            "You do not have enough space to load the packages. You need to make room for %d more tonne.",
            "You do not have enough space to load the packages. You need to make room for %d more tonnes.",
            needed), needed ) )
         return
      end

      -- Update mission
      local c = misn.cargoNew(N_("Packages"), N_("Several packages of \"food\"."))
      mem.packages = misn.cargoAdd(c, 3)
      mem.misn_stage = 1
      mem.jumped = 0
      misn.setDesc( fmt.f(_("Deliver the package to {pnt} in the {sys} system"), {pnt=mem.dest, sys=mem.destsys}) )
      misn.markerMove( mem.misn_marker, mem.destsys )
      misn.osdCreate(_("Empire Shipping Delivery"), {
         fmt.f(_("Deliver the package to {pnt} in the {sys} system"), {pnt=mem.dest, sys=mem.destsys}),
      })

      -- Load message
      tk.msg( _("Loading Cargo"), fmt.f( _([[The packages labelled "Food" are loaded discreetly onto your ship. Now to deliver them to {pnt} in the {sys} system.]]), {pnt=mem.dest, sys=mem.destsys}) )

   elseif mem.landed == mem.dest and mem.misn_stage == 1 then
      if misn.cargoRm(mem.packages) then
         -- Update mission
         mem.misn_stage = 2
         misn.setDesc( fmt.f(_("Return to {pnt} in the {sys} system"), {pnt=mem.ret, sys=mem.retsys}) )
         misn.markerMove( mem.misn_marker, mem.retsys )
         misn.osdCreate(_("Empire Shipping Delivery"), {fmt.f(_("Return to {pnt} in the {sys} system"), {pnt=mem.ret, sys=mem.retsys})})

         -- Some text
         tk.msg( _("Cargo Delivery"), fmt.f(_([[Workers quickly unload the package as mysteriously as it was loaded. You notice that one of them gives you a note. Looks like you'll have to go to {pnt} in the {sys} system to report to Commander Soldner.]]), {pnt=mem.ret, sys=mem.retsys}) )
      end
   elseif mem.landed == mem.ret and mem.misn_stage == 2 then

      -- Rewards
      player.pay( emp.rewards.es01 )
      faction.modPlayerSingle("Empire",5);

      -- Flavour text
      tk.msg(_("Mission Success"), fmt.f(_([[You arrive at {pnt} and report to Commander Soldner. He greets you and starts talking. "I heard you encountered resistance. At least you managed to deliver the package. Great work there. I've managed to get you cleared for the Heavy Weapon License. You'll still have to pay the fee for getting it, though.
    "If you're interested in more work, meet me in the bar in a bit. I've got some paperwork I need to finish first."]]), {pnt=mem.ret}) )

      -- The goods
      diff.apply("heavy_weapons_license")

      emp.addShippingLog( _([[You successfully completed a package delivery for the Empire. As a result, you have been cleared for the Heavy Weapon License and can now buy it at an outfitter. Commander Soldner said that you can meet him in the bar at Halir if you're interested in more work.]]) )

      misn.finish(true)
   end
end


function enter ()
   mem.sys = system.cur()

   if mem.misn_stage == 1 then

      -- Mercenaries appear after a couple of jumps
      mem.jumped = mem.jumped + 1
      if mem.jumped <= 3 then
         return
      end

      -- Get player position
      mem.enter_vect = player.pos()

      -- Calculate where the enemies will be
      local r = rnd.rnd(0,4)
      -- Next to player (always if landed)
      if mem.enter_vect:dist() < 1000 or r < 2 then
         local a = rnd.rnd() * 360
         local d = rnd.rnd( 400, 1000 )
         mem.enter_vect:add( vec2.newP( d, a ) )
         enemies()
      -- Enter after player
      else
         hook.timer(rnd.uniform( 2.0, 5.0 ) , "enemies")
      end
   end
end


function enemies ()
   -- Choose mercenaries
   local merc = {}
   if rnd.rnd() < 0.3 then table.insert( merc, "Pacifier" ) end
   if rnd.rnd() < 0.7 then table.insert( merc, "Ancestor" ) end
   if rnd.rnd() < 0.9 then table.insert( merc, "Vendetta" ) end

   -- Add mercenaries
   for k,v in ipairs(merc) do
      -- Move position a bit
      local a = rnd.rnd() * 360
      local d = rnd.rnd( 50, 75 )
      mem.enter_vect:add( vec2.newP( d, a ) )
      -- Add pilots
      local p = pilot.add( v, "Mercenary", mem.enter_vect )
      p:setHostile()
   end
end


