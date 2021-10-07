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

   if tbroad == "afterburner" and not var.peek( "tut_afterburner" ) then
      vn.clear()
      vn.scene()
      local sai = vn.newCharacter( tut.vn_shipai() )
      vn.transition( tut.shipai.transition )
      sai(fmt.f(_([["Looks like you just bought an #oafterburner#0 outfit. Afterburners are active outfits that you can only equip one of on any specific ship. You can either configure to be set to a weapon set keybinding in the Info menu (opened with {infokey}, or double tap {accelkey} to trigger it. Note that they are prone to overheating and use a lot of energy, so it is bes to use afterburners only when necessary."]]),{infokey=tut.getKey("info"), accelkey=tut.getKey("accel")}))
      vn.done( tut.shipai.transition )
      vn.run()
      var.push( "tut_afterburner" )
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
