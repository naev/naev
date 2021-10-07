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

      tk.msg( "", _([[The person who sells you the %s looks at your record and pauses. "Ah, I see you haven't owned a large ship before! Sorry to slow you down, but I just wanted to tell you some important things about your ship. I promise I'll be but a moment.
   "Firstly, you may notice that the ship you bought has a 'Time Constant' rating. See, when operating a larger ship, you have to expend more time and effort performing the basic operations of the ship, causing your perception of time to speed up. Time Constant is simply a measure of how fast you will perceive the passage of time compared to a typical small ship; for example, a Time Constant rating of 200%% means that time appears to pass twice as fast as typical small ships.
   "This, and the slower speed of your ship, may make it difficult to use forward-facing weapons as well as on smaller ships. For the largest classes - Destroyers and up - I would generally recommend use of turreted weapons, which will automatically aim at your opponent, rather than forward-facing weapons. That's of course up to you, though.
   "That's all! Sorry to be a bother. I wish you good luck in your travels!" You thank the salesperson and continue on your way.]]):format(shp) )
      var.push( "tut_timedil", true )
      return
   end
end

function takeoff ()
   evt.finish()
end
