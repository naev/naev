local fmt = require "format"
local lib = {}
function lib.setup( costfunc )
   notactive = true -- Doesn't become active

   function price( _q )
      local c = costfunc()
      local canbuy = (c <= player.credits())
      return fmt.credits(c), canbuy, false
   end

   function buy( _q )
      local c = costfunc()
      if c > player.credits() then
         return false, _("You do not have enough credits to purchase this item.")
      end
      player.pay( -c )
      return true, 1 -- Can only buy one at a time
   end

   function sell( _q )
      return false, _("You can not sell this outfit.")
   end

   function descextra ( _p, _o, _po )
      return "#o".._("This outfit can not be removed once equipped.").."#0"
   end
end
return lib
