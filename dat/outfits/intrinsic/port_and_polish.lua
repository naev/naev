notactive = true -- Doesnt' become active

local fmt = require "format"

local function cost ()
   local pp = player.pilot()
   local size = pp:ship():size()
   return 25e3 * math.pow(2+size, 2)
end

function price( _q )
   local c = cost()
   local canbuy = (c <= player.credits())
   return fmt.credits(c), canbuy, false
end

function buy( _q )
   local c = cost()
   if c > player.credits() then
      return false, _("You do not have enough credits to purchase this item.")
   end
   player.pay( -c )
   return true, 1 -- Can only buy one at a time
end

function sell( _q )
   return false, _("You can not sell this outfit.")
end
