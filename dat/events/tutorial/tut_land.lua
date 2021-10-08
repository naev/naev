--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Land Tutorial Event">
 <trigger>land</trigger>
 <chance>100</chance>
</event>
--]]
--[[

   Tutorial Event

--]]
local fmt = require "format"
local tut = require "common.tutorial"

function create ()
   if tut.isDisabled() then evt.finish() end

   hook.outfit_buy( "outfit_buy" )
   hook.ship_buy( "ship_buy" )
   hook.takeoff( "takeoff" )
end

function outfit_buy( outfitname )
   local o = outfit.get(outfitname)
   local tbroad = o:typeBroad()

   -- TODO tutorials when buying licenses

   if tbroad == "Afterburner" and not var.peek( "tut_afterburner" ) then
      vn.clear()
      vn.scene()
      local sai = vn.newCharacter( tut.vn_shipai() )
      vn.transition( tut.shipai.transition )
      sai(fmt.f(_([["Looks like you just bought an #oAfterburner#0 outfit. Afterburners are active outfits that you can only equip one of on any specific ship. You can either configure to be set to a weapon set keybinding in the Info menu (opened with {infokey}, or double tap {accelkey} to trigger it. Note that they are prone to overheating and use a lot of energy, so it is bes to use afterburners only when necessary."]]),{infokey=tut.getKey("info"), accelkey=tut.getKey("accel")}))
      vn.done( tut.shipai.transition )
      vn.run()
      var.push( "tut_afterburner" )

   elseif tbroad == "Launcher" and not var.peek( "tut_launcher" ) then
      vn.clear()
      vn.scene()
      local sai = vn.newCharacter( tut.vn_shipai() )
      vn.transition( tut.shipai.transition )
      sai(_([["Looks like you just acquired your first #oLauncher#0 outfit. Launchers are ammo-based weapons that can have target tracking abilities. Seeking launchers have two important properties: '#oLock-on#0' and '#oIn-Flight Calibration#0'. Lock-on determines how many seconds it takes to be able to launch rockets after getting a new target. It is modulated depending if the target's '#oEvasion#0' is lower than the '#oOptimal Tracking#0'."]]))
      sai(_([["'#oIn-Flight Calibration#0' is the amount of time it takes for the rocket after being launched to start tracking the target. When not locked-on it will fly in a straight line and damage all hostiles they encounter, however, once locked-on they will only damage the target unless jammed. In-Flight Calibration is not affected by the target's evasion, and is visualized by a coloured circle around the outfit that shrinks as the calibration finishes.."]]))
      sai(_([["That leads us to the last important concept: jamming! Seeking rockets can be jammed depending on whether or not the target has jamming equipment, and the resistance of the rocket. The chance of a rocket being jammed is the difference between the jamming chance and the resistance. When a rocket becomes jammed it can either get slowed down, or get stuck in a random trajectory. Despite being jammed, they can still damage any hostiles they encounter."]]))
      sai(fmt.f(_([["Finally, the ammo of launchers regenerates over time: there is no need to buy ammunition. By either performing a cooldown with {cooldownkey} or double-tapping {reversekey}, or landing on a planet or station you can instantly refill the ammunition when necessary. Launchers can be very useful if you master them, please try them out with different configurations!"]]),{cooldownkey=tut.getKey("cooldown"), reversekey=tut.getKey("reverse")}))
      vn.done( tut.shipai.transition )
      vn.run()
      var.push( "tut_launcher" )

   elseif tbroad == "Fighter Bay" and not var.peek( "tut_fighterbay" ) then
      vn.clear()
      vn.scene()
      local sai = vn.newCharacter( tut.vn_shipai() )
      vn.transition( tut.shipai.transition )
      sai(fmt.f(_([["Looks like you are moving up in the world with your first #oFighter Bay#0 outfit! As you can expect, fighter bays let you launch and control interceptor or fighter class escorts. They autonomously will defend your ship when deployed, and can be given particular orders. You can tell them to #oattack#0 your target with {eattackkey}, #oclear orders#0 with {eclearkey}, #ohold position#0 with {eholdkey}, and #oreturn to ship#0 with {ereturnkey}."]]),{eattackkey=tut.getKey("e_attack"), eholdkey=tut.getKey("e_hold"), ereturnkey=tut.getKey("e_return"), eclearkey=tut.getKey("e_clear")}))
      sai(fmt.f(_([["Like ammunition in launchers, lost fighters will regenerate slowly over time, and you can restock them by either performing a cooldown operation with {cooldownkey} or double-tapping {reversekey}, or landing on a planet or station. You can either fly around with deployed fighters or keep them inside your ship and only deploy as necessary. You should try to see whatever works best for you!"]]),{cooldownkey=tut.getKey("cooldown"), reversekey=tut.getKey("reverse")}))
      vn.done( tut.shipai.transition )
      vn.run()
      var.push( "tut_fighterbay" )
   end
end

function ship_buy( shipname )
   local s = ship.get(shipname)

   if not var.peek( "tut_buyship" ) then
      vn.clear()
      vn.scene()
      local sai = vn.newCharacter( tut.vn_shipai() )
      vn.transition( tut.shipai.transition )
      sai(fmt.f(_([["Congratulations on buying a brand new {shipname}! Unless you trade-in your ship, when you buy a new ship it is added to your available ships. Usually, ships come with only core outfits quipped, so you should head over to the #oEquipment#0 window to deck the ship out and swap it with your current one if you want to use it. You can swap ships at any planet or station with a shipyard and there is no penalty nor cost associated with swapping. In fact, getting a diversity of ships and switching to the one that best fits your need is a great way to get things done."]]),{shipname=shipname}))
      sai(_([["You also don't have to worry about your ship AI changing, I am automatically transferred between your ships. You can't get away from me, ha ha."]]))
      vn.done( tut.shipai.transition )
      vn.run()
      var.push( "tut_buyship" )
   end

   if s:time_mod() > 1 and not var.peek( "tut_timedil" ) then
      vn.clear()
      vn.scene()
      local sai = vn.newCharacter( tut.vn_shipai() )
      vn.transition( tut.shipai.transition )
      sai(_([["Whoa, it looks like you just acquired your first large ship. You may have noticed by now, but ships have a '#oTime Constant#0' rating, which indicates how time feels to pass by when flying the ship. Smaller ships tend to have values below 100%, and thus time ends up passing slower in them than in other ships. On the other hand, large ships will have time constants above 100%, making time seem to pass faster. For example, a ship with a 150% time constant will have time passing 50% faster than a normal ship, and will help the ship feel less cumbersome to fly."]]))
      sai(_([["In general, due to the low maneuverability of large ships, you should consider using turret weapons and/or fighter bays for self defense. They allow you to worry less about aiming, and more about getting jobs done. Turrets rely heavily on their tracking values. '#oMinimum Tracking#0' refers to the '#oEvasion#0' value of a target ship at which the turret will not be able to track. On the other hand, if the target ship is above the '#oOptimal Tracking#0', your turrets will be able to perfectly track them. It is best to try out different turrets and see what feels best for your ship."]]))
      vn.done( tut.shipai.transition )
      vn.run()
      var.push( "tut_timedil", true )
      return
   end
end

function takeoff ()
   evt.finish()
end
