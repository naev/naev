local der = require 'common.derelict'
local vn = require 'vn'
local fmt = require 'format'
local jlib = require "common.junker"

local OUTFIT = outfit.get("Junker Pack")

return function ()
   -- Have gotten the pack already
   if player.outfitNum(OUTFIT) <= 0 then
      return
   end
   -- Must be a system the junker likes
   if not jlib.good_sys() then
      return
   end
   -- Must have not finished event already
   if player.evtDone("Meet the Junker") then
      return
   end

   return {
      weight = 2,
      func = function ()
         vn.clear()
         vn.scene()
         vn.sfx( der.sfx.board )
         vn.music( der.sfx.ambient )
         vn.transition()

         vn.na(_([[You board the derelict and suddenly feel a sense of déjà vu: the ship has been stripped clean in a meticulous way you've only seen once before. How strange...]]))
         vn.na(_([[Wait, you get a signal that it seems like a ship is rapidly your location. A pirate trap?!?]]))
         vn.na(_([[You rush back aboard your ship before you get blown up into smithereens.]]))

         vn.sfx( der.sfx.unboard )
         vn.run()
         player.unboard()

         der.addMiscLog(fmt.f(_([[You boarded a derelict in the {sys} system and were ambushed by a strange pilot known as "The Junker".]]),
            {sys = system.cur()}))

         naev.eventStart("Meet the Junker")
         return true
      end
   }
end
