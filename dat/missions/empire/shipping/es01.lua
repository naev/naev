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
   pickup,pickupsys  = planet.getLandable( "Selphod" )
   dest,destsys      = planet.getLandable( "Cerberus" )
   ret,retsys        = planet.getLandable( "Halir" )
   if pickup==nil or dest==nil or ret==nil then
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
   misn_marker       = misn.markerAdd( pickupsys, "low" )

   -- Mission details
   misn_stage = 0
   reward = 500e3
   misn.setTitle(_("Empire Shipping Delivery"))
   misn.setReward( fmt.credits(reward) )
   misn.setDesc( string.format(_("Pick up a package at %s in the %s system"), pickup:name(), pickupsys:name()))

   -- Flavour text and mini-briefing
   tk.msg( _("Commander Soldner"), string.format( _([[Commander Soldner begins, "We have an important package that we must take from %s in the %s system to %s in the %s system. We have reason to believe that it is also wanted by external forces.
    "The plan is to send an advance convoy with guards to make the run in an attempt to confuse possible enemies. You will then go in and do the actual delivery by yourself. This way we shouldn't arouse suspicion. You are to report here when you finish delivery and you'll be paid %s."]]), pickup:name(), pickupsys:name(),
         dest:name(), destsys:name(), fmt.credits(reward) ) )
   misn.osdCreate(_("Empire Shipping Delivery"), {_("Pick up a package at %s in the %s system"):format(pickup:name(), pickupsys:name())})

   -- Set up the goal
   tk.msg( _("Commander Soldner"), _([["Avoid hostility at all costs. The package must arrive at its destination. Since you are undercover, Empire ships won't assist you if you come under fire, so stay sharp. Good luck."]]) )

   -- Set hooks
   hook.land("land")
   hook.enter("enter")
end


function land ()
   landed = planet.cur()

   if landed == pickup and misn_stage == 0 then

      -- Make sure player has room.
      if player.pilot():cargoFree() < 3 then
         local needed = 3 - player.pilot():cargoFree()
         tk.msg( _("Need More Space"), string.format( gettext.ngettext(
            "You do not have enough space to load the packages. You need to make room for %d more tonne.",
            "You do not have enough space to load the packages. You need to make room for %d more tonnes.",
            needed), needed ) )
         return
      end

      -- Update mission
      local c = misn.cargoNew(_("Packages"), _("Several packages of \"food\"."))
      packages = misn.cargoAdd(c, 3)
      misn_stage = 1
      jumped = 0
      misn.setDesc( string.format(_("Deliver the package to %s in the %s system"), dest:name(), destsys:name()))
      misn.markerMove( misn_marker, destsys )
      misn.osdCreate(_("Empire Shipping Delivery"), {_("Deliver the package to %s in the %s system"):format(dest:name(), destsys:name())})

      -- Load message
      tk.msg( _("Loading Cargo"), string.format( _([[The packages labelled "Food" are loaded discreetly onto your ship. Now to deliver them to %s in the %s system.]]), dest:name(), destsys:name()) )

   elseif landed == dest and misn_stage == 1 then
      if misn.cargoRm(packages) then
         -- Update mission
         misn_stage = 2
         misn.setDesc( string.format(_("Return to %s in the %s system"), ret:name(), retsys:name()))
         misn.markerMove( misn_marker, retsys )
         misn.osdCreate(_("Empire Shipping Delivery"), {_("Return to %s in the %s system"):format(ret:name(),retsys:name())})

         -- Some text
         tk.msg( _("Cargo Delivery"), string.format(_([[Workers quickly unload the package as mysteriously as it was loaded. You notice that one of them gives you a note. Looks like you'll have to go to %s in the %s system to report to Commander Soldner.]]), ret:name(), retsys:name()) )
      end
   elseif landed == ret and misn_stage == 2 then

      -- Rewards
      player.pay(reward)
      faction.modPlayerSingle("Empire",5);

      -- Flavour text
      tk.msg(_("Mission Success"), string.format(_([[You arrive at %s and report to Commander Soldner. He greets you and starts talking. "I heard you encountered resistance. At least you managed to deliver the package. Great work there. I've managed to get you cleared for the Heavy Weapon License. You'll still have to pay the fee for getting it, though.
    "If you're interested in more work, meet me in the bar in a bit. I've got some paperwork I need to finish first."]]), ret:name()) )

      -- The goods
      diff.apply("heavy_weapons_license")

      emp.addShippingLog( _([[You successfully completed a package delivery for the Empire. As a result, you have been cleared for the Heavy Weapon License and can now buy it at an outfitter. Commander Soldner said that you can meet him in the bar at Halir if you're interested in more work.]]) )

      misn.finish(true)
   end
end


function enter ()
   sys = system.cur()

   if misn_stage == 1 then

      -- Mercenaries appear after a couple of jumps
      jumped = jumped + 1
      if jumped <= 3 then
         return
      end

      -- Get player position
      enter_vect = player.pos()

      -- Calculate where the enemies will be
      r = rnd.rnd(0,4)
      -- Next to player (always if landed)
      if enter_vect:dist() < 1000 or r < 2 then
         a = rnd.rnd() * 2 * math.pi
         d = rnd.rnd( 400, 1000 )
         enter_vect:add( math.cos(a) * d, math.sin(a) * d )
         enemies()
      -- Enter after player
      else
         t = hook.timer(rnd.uniform( 2.0, 5.0 ) , "enemies")
      end
   end
end


function enemies ()
   -- Choose mercenaries
   merc = {}
   if rnd.rnd() < 0.3 then table.insert( merc, "Pacifier" ) end
   if rnd.rnd() < 0.7 then table.insert( merc, "Ancestor" ) end
   if rnd.rnd() < 0.9 then table.insert( merc, "Vendetta" ) end

   -- Add mercenaries
   for k,v in ipairs(merc) do
      -- Move position a bit
      a = rnd.rnd() * 2 * math.pi
      d = rnd.rnd( 50, 75 )
      enter_vect:add( math.cos(a) * d, math.sin(a) * d )
      -- Add pilots
      p = pilot.add( v, "Mercenary", enter_vect )
      p:setHostile()
   end
end


