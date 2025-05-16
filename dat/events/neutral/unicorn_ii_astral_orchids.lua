--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Unicorn II Astral Orchids">
 <location>land</location>
 <chance>100</chance>
 <spob>Unicorn II</spob>
 <unique />
</event>
--]]
local vn = require "vn"
local fmt = require "format"
local tut = require "common.tutorial"

local VARNAME = "unicorn_ii_astral_orchids"
local REGROW_TIME = time.new( 0, 1200, 0 )
local HARVEST_AMOUNT = 2
local CROP = commodity.get("Astral Nectar")

function create ()
   local lastpicked = var.peek( VARNAME )
   local canpick = (not lastpicked) or (lastpicked >= time.get() + REGROW_TIME)

   vn.clear()
   vn.scene()

   if not canpick then
      -- Sorry player
      vn.transition()
      vn.na(_([[You land and go check on the Astral Orchids you found last times. It seems like not enough time has passed for the nectar to regrow.]]))
      vn.done()
      vn.run()
      return evt.finish()
   elseif lastpicked~=nil then
      -- Skip intro stuff
      vn.transition()
      vn.na(_([[You land and go check on the Astral Orchids you found last time. It seems like they are ready to harvest again.]]))
      vn.menu{
         {_([[Harvest the Orchids.]]), "02_harvest"},
         {_([[Let them be.]]), "02_leave"},
      }
   end

   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )
   vn.na(fmt.f(_([[Your ship lands, and you run a preliminary scan, when {ainame} pops up.]]),
      {ainame=tut.ainame()}))
   sai(_([["I have detected some organic material in a nearby cave which may be of interest. Or not. I lack the statistical priors to estimate your response to such data."]]))

   vn.menu{
      {_([[Explore the caves.]]), "01_explore"},
      {_([[Maybe next time.]]), "01_maybenext"},
   }

   vn.label("01_maybenext")
   vn.done()

   vn.label("01_explore")
   vn.na(fmt.f(_([[You don your atmospheric suit and take a weapon with you as you leave your ship, just in case. You head towards the coordinates provided by {ainame}.]]),
      {ainame=tut.ainame()}))
   vn.na(_([[You find the entrance to a cave nearby, and start exploring the subterranean world. Eventually, you find an expansive cavern, and your sensors pick up nearby organic matter. Looking carefully, you find some small plants that seem to be flowering, what are tho odds?]]))
   sai(fmt.f(_([["These seem to be {crop}, a rare species that requires specific conditions to thrive, and do rely on chemosynthesis for survival. They are quite rare and highly sought after by gourmets for their nectar."]]),
      {crop="#b".._("Astral Orchids").."#0"}))
   sai(_([["However, the planet was deemed to be extinct. It is incredible that you were able to find wild ones growing in such a remote location."]]))
   vn.na(_([[What do you want to do?]]))
   vn.menu{
      {_([[Harvest the Orchids.]]), "02_harvest"},
      {_([[Let them be.]]), "02_leave"},
   }

   vn.label("02_leave")
   vn.na(_([[You let the Astral Orchids be, and make your way back to your ship. Some things are best left alone.]]))
   vn.done( tut.shipai.transition )

   vn.label("02_harvest")
   local harvested = 0
   vn.func( function ()
      if player.fleetCargoFree() <= 0 then
         return vn.jump("nofit")
      end
      harvested = player.fleetCargoAdd( CROP, HARVEST_AMOUNT )
      var.push( VARNAME, time.get() )
      time.inc( time.new( 0, 1, 0 ) )
   end )
   vn.na( function ()
      return fmt.f(_([[You carefully collect {amount} of {crop} and bring it back to your ship. It seems like it'll likely take a while to grow back.]]),
         {amount=fmt.tonnes(harvested), crop=CROP})
   end )
   vn.done( tut.shipai.transition )

   vn.label("nofit")
   vn.na(_([[You realize you don't have the cargo space to fit what you could collect. It looks like you'll have to come back later.]]))
   if lastpicked==nil then
      vn.done()
   else
      vn.done( tut.shipai.transition )
   end

   vn.run()

   evt.finish()
end
