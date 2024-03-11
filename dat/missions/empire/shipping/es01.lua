--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Empire Shipping 2">
 <unique />
 <priority>2</priority>
 <cond>
   if faction.playerStanding("Empire") &lt; 0 or faction.playerStanding("Dvaered") &lt; 0 or faction.playerStanding("FLF") &gt;= 10 then
      return false
   end
   return true
 </cond>
 <chance>100</chance>
 <done>Empire Shipping 1</done>
 <location>Bar</location>
 <spob>Halir</spob>
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
local fleet = require "fleet"
local fmt = require "format"
local emp = require "common.empire"
local vn = require "vn"
local vntk = require "vntk"

function create ()
   -- Planet targets
   mem.pickup,mem.pickupsys  = spob.getS( "Selphod" )
   mem.dest,mem.destsys      = spob.getS( "Cerberus Outpost" )
   mem.ret,mem.retsys        = spob.getS( "Halir" )
   -- Note: this mission does not make any system claims

   -- Bar NPC
   misn.setNPC( emp.soldner.name, emp.soldner.portrait, emp.soldner.description )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local soldner = vn.newCharacter( emp.vn_soldner() )
   vn.transition( emp.soldner.transition )

   -- See if accept mission
   soldner(_([[You approach Commander Soldner, who seems to be waiting for you.
"Ready for your next mission? We need you to do a pickup and delivery, however, you are expected to encounter hostilities on the way. Interested?"]]) )
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Decline]]), "decline"},
   }

   vn.label("decline")
   vn.done( emp.soldner.transition )

   -- Flavour text and mini-briefing
   vn.label("accept")
   soldner(fmt.f(_([[Commander Soldner begins, "We have an important package that must get from {pickup_pnt} in the {pickup_sys} system to {dropoff_pnt} in the {dropoff_sys} system. We have reason to believe that it is also wanted by external forces.]]),
      {pickup_pnt=mem.pickup, pickup_sys=mem.pickupsys, dropoff_pnt=mem.dest, dropoff_sys=mem.destsys}))
   soldner(fmt.f(_([["The plan is to send an advance convoy with guards to make the run in an attempt to confuse possible enemies. You will then go in and do the actual delivery by yourself. This way we shouldn't arouse suspicion. You are to report here when you finish delivery, and you'll be paid {credits}."]]),
      {credits=fmt.credits(emp.rewards.es01)} ) )
   soldner(_([["Avoid hostilities at all costs. The package must arrive at its destination. Since you are undercover, Empire ships won't assist you if you come under fire, so stay sharp. Good luck."]]))
   vn.func( function () accepted = true end )

   vn.done( emp.soldner.transition )
   vn.run()

   if not accepted then return end

   misn.accept()

   -- target destination
   mem.misn_marker       = misn.markerAdd( mem.pickup, "low" )

   -- Mission details
   mem.misn_stage = 0
   misn.setTitle(_("Empire Shipping Delivery"))
   misn.setReward( emp.rewards.es01 )
   misn.setDesc( fmt.f(_("Pick up a package at {pnt} in the {sys} system"), {pnt=mem.pickup, sys=mem.pickupsys}) )
   misn.osdCreate(_("Empire Shipping Delivery"), {
      fmt.f(_("Pick up a package at {pnt} in the {sys} system"), {pnt=mem.pickup, sys=mem.pickupsys}),
   })

   -- Set hooks
   hook.land("land")
   hook.enter("enter")
end

function land ()
   mem.landed = spob.cur()

   if mem.landed == mem.pickup and mem.misn_stage == 0 then

      -- Make sure player has room.
      if player.pilot():cargoFree() < 3 then
         local needed = 3 - player.pilot():cargoFree()
         vntk.msg( _("Need More Space"), string.format( n_(
            "You do not have enough space to load the packages. You need to make room for %d more tonne.",
            "You do not have enough space to load the packages. You need to make room for %d more tonnes.",
            needed), needed ) )
         return
      end

      -- Update mission
      local c = commodity.new(N_("Packages"), N_("Several packages of \"food\"."))
      mem.packages = misn.cargoAdd(c, 3)
      mem.misn_stage = 1
      mem.jumped = 0
      misn.setDesc( fmt.f(_("Deliver the package to {pnt} in the {sys} system"), {pnt=mem.dest, sys=mem.destsys}) )
      misn.markerMove( mem.misn_marker, mem.dest )
      misn.osdCreate(_("Empire Shipping Delivery"), {
         fmt.f(_("Deliver the package to {pnt} in the {sys} system"), {pnt=mem.dest, sys=mem.destsys}),
      })

      -- Load message
      vntk.msg( _("Loading Cargo"), fmt.f( _([[The packages labelled "Food" are loaded discreetly onto your ship. Now to deliver them to {pnt} in the {sys} system.]]), {pnt=mem.dest, sys=mem.destsys}) )

   elseif mem.landed == mem.dest and mem.misn_stage == 1 then
      if misn.cargoRm(mem.packages) then
         -- Update mission
         mem.misn_stage = 2
         misn.setDesc( fmt.f(_("Return to {pnt} in the {sys} system"), {pnt=mem.ret, sys=mem.retsys}) )
         misn.markerMove( mem.misn_marker, mem.ret )
         misn.osdCreate(_("Empire Shipping Delivery"), {fmt.f(_("Return to {pnt} in the {sys} system"), {pnt=mem.ret, sys=mem.retsys})})

         -- Some text
         vntk.msg( _("Cargo Delivery"), fmt.f(_([[Workers quickly unload the package as discreetly as it was loaded. You notice that one of them gives you a note. Looks like you'll have to go to {pnt} in the {sys} system to report to Commander Soldner.]]), {pnt=mem.ret, sys=mem.retsys}) )
      end
   elseif mem.landed == mem.ret and mem.misn_stage == 2 then

      -- Rewards
      local getlicense = not diff.isApplied( "heavy_weapons_license" )

      vn.clear()
      vn.scene()
      local soldner = vn.newCharacter( emp.vn_soldner() )
      vn.transition( emp.soldner.transition )

      -- Flavour text
      if getlicense then
         soldner(fmt.f(_([[You arrive at {pnt} and report to Commander Soldner. He greets you and starts talking. "I heard you encountered resistance. At least you were able to deliver the package. Great work there. I've managed to get you cleared for a Heavy Weapon License. You'll still have to pay the fee for getting it, though.]]),
            {pnt=mem.ret}))
         vn.sfxBingo()
         vn.na(_([[You can now purchase the #bHeavy Weapon License#0.]]))
      else
         soldner(fmt.f(_([[You arrive at {pnt} and report to Commander Soldner. He greets you and starts talking. "I heard you encountered resistance. At least you managed to deliver the package."]]),
            {pnt=mem.ret}))
      end
      soldner(_([["If you're interested in more work, meet me in the bar in a bit. I've got some paperwork I need to finish first."]]))

      vn.sfxVictory()
      vn.func( function ()
         player.pay( emp.rewards.es01 )
         faction.modPlayerSingle("Empire",5)
      end )
      vn.na( fmt.reward(emp.rewards.es01) )

      vn.done( emp.soldner.transition )
      vn.run()

      -- The goods
      if getlicense then
         diff.apply("heavy_weapons_license")
         emp.addShippingLog( _([[You successfully completed a package delivery for the Empire. As a result, you have been cleared for a Heavy Weapon License and can now buy it at an outfitter. Commander Soldner said that you can meet him in the bar at Halir if you're interested in more work.]]) )
      else
         emp.addShippingLog( _([[You successfully completed a package delivery for the Empire. Commander Soldner said that you can meet him in the bar at Halir if you're interested in more work.]]) )
      end

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
      local enter_vect = player.pos()

      -- Calculate where the enemies will be
      local r = rnd.rnd(0,4)
      -- Next to player (always if landed)
      if enter_vect:dist() < 1000 or r < 2 then
         enter_vect:add( vec2.newP( rnd.rnd(400, 1000), rnd.angle() ) )
         enemies( enter_vect )
      -- Enter after player
      else
         hook.timer(rnd.uniform( 2.0, 5.0 ) , "enemies", enter_vect)
      end
   end
end

function enemies( enter_vect )
   -- Choose mercenaries
   local merc = {}
   if rnd.rnd() < 0.3 then table.insert( merc, "Pacifier" ) end
   if rnd.rnd() < 0.7 then table.insert( merc, "Ancestor" ) end
   if rnd.rnd() < 0.9 then table.insert( merc, "Vendetta" ) end

   -- Add mercenaries
   local flt = fleet.add( 1, merc, "Mercenary", enter_vect )
   for k,p in ipairs(flt) do
      p:setHostile()
   end
end
