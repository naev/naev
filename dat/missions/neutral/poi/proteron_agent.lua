--local fmt = require "format"
local vn = require "vn"

return function ( _mem )
   return {
      type = "function",
      func = function ()
         vn.na(_([[You explore the ship and eventually reach the systems room. You notice there seems to be a device interfering with the radiation emitted. You can't tell who made it, but it seems that it was likely the main reason that the derelict was so hard to find. You manage to dislodge it to take it back to your ship for further analysis.]]))
         vn.func( function ()
            player.outfitAdd( "Veil of Penelope" )
         end )
      end,
   }
end
