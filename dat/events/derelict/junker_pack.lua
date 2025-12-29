local der = require 'common.derelict'
local vn = require 'vn'
local fmt = require 'format'
local jlib = require "common.junker"

local OUTFIT = outfit.get("Junker Pack")

return function ()
   -- Must have not gotten the pack yet
   if player.outfitNum(OUTFIT) > 0 then
      return
   end
   -- Must be a system the junker likes
   if not jlib.good_sys() then
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

         vn.na(_([[You board the derelict ship and begin a routine search to see if there's anything left. Unluckily, it seems like the ships has been picked clean in a surprisingly methodological way. Almost as if someone went room by room, taking apart the most valuable parts, before progressing to the next room. The precision of the disassembly is uncanny, and something you haven't seen before in your travels.]]))
         vn.na(_([[You go about the ship, about to give up on finding anything useful, when you find a weird mass of different ship parts sort of mashed together. In fact, not only seemingly random ship parts, some of quite high value. Not wanting it to go to waste, you decide to take it with you and leave the ship.]]))
         vn.func( function ()
            player.outfitAdd(OUTFIT)
         end )
         vn.sfxBingo()
         vn.na(fmt.reward(OUTFIT))

         vn.sfx( der.sfx.unboard )
         vn.run()
         player.unboard()

         der.addMiscLog(fmt.f(_("You found a meticulously cleaned up derelict in the {sys} system with some weird Junker Pack that someone abandoned or perhaps forgot. Seeming rather useful, you took it with you."),
            {sys = system.cur()}))

         return true
      end
   }
end
