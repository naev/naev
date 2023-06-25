--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Pirate Fake Transponder">
  <unique/>
 <location>land</location>
 <cond>require("common.pirate").factionIsPirate( spob.cur():faction() ) and faction.playerStanding("Pirate") &gt;= -20 and player.credits() &gt;= 500e3</cond>
 <chance>50</chance>
</event>
--]]
--[[
   Pirate offers to sell the player a fake transponder. Might be better to have it be a small mission or campaign to be a bit more interesting given that it should be fairly strong.
--]]
local vn = require 'vn'
local vni = require 'vnimage'
local fmt = require "format"

local pir_name = _("Shifty-Eyed Pirate")
local pir_image, pir_portrait = vni.pirate()
local pir_description = _("You see a seedy pirate flashing looks at you, as if they had something interesting to show you.")

local transponder = outfit.get("Fake Transponder")
local cost = 1e6

function create ()
   -- Player already has it somehow, so this event makes no sense
   if player.numOutfit(transponder) > 0 then
      evt.finish(true)
   end

   evt.npcAdd( "approach_pirate", pir_name, pir_portrait, pir_description )
   hook.enter( "enter" )
end

function approach_pirate ()
   vn.clear()
   vn.scene()
   local p = vn.newCharacter( pir_name, {image=pir_image} )
   vn.transition()
   vn.na(_([[You approach the pirate who begins grinning from ear to ear.]]))
   p(fmt.f(_([["You look like you know a steal when you see one. I've got my hands on a marvel of technology, a #o{outfit}#0 that can help you avoid all those pesky ships when basking in the joys of piracy. No matter what ship you're flying, sensors will pick you up as a docile and inoffensive independent ship. It can fool pretty much any ship sensors!"]]),{outfit=transponder}))
   p(fmt.f(_([["As a one-time offer, I'd be willing to part with it for {credits}. It's such a steal that it's almost like you're robbing me!"]]),{credits=fmt.credits(cost)}))

   vn.menu{
      {_("Purchase the Transponder"), "trybuy"},
      {_("Leave"), "leave"},
   }

   vn.label("broke")
   vn.na(_([[You lack the funds to make the purchase and the pirate quickly loses interest in you.]]))
   vn.done()

   vn.label("trybuy")
   vn.func( function ()
      if player.credits() < cost then
         vn.jump("broke")
         return
      end
      player.pay( -cost )
      player.outfitAdd( transponder )
   end )
   vn.na( fmt.reward(transponder:name()) )
   p(_([["You won't regret this purchase! Just make sure to not get scanned, or they'll find out who you are."
Having sold his wares, the pirate disappears into the shadows.]]))
   vn.done()

   vn.label("leave")
   vn.na(_([[You leave the pirate and their fake transponder behind.]]))
   vn.run()

   -- Player bought it, we're done!
   if player.numOutfit(transponder) > 0 then
      evt.finish(true)
   end
end

function enter ()
   evt.finish()
end
