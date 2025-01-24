notactive = true -- Doesn't become active

local fmt = require "format"

local function cost ()
   local pp = player.pilot()
   local size = pp:ship():size()
   return 30e3 * math.pow(1+size, 2)
end

local FUEL_SMALL = 100
local FUEL_MEDIUM = 200
local FUEL_LARGE = 400

local function fuel_from_size( p )
   local s = p:size()
   if s<=2 then
      return FUEL_SMALL
   elseif s<=4 then
      return FUEL_MEDIUM
   else
      return FUEL_LARGE
   end
end

function descextra( p, _o )
   local function addstr( fuel, size, hilight, nocol )
      local s = fmt.f(_("Provides {fuel} fuel to {size} ships."),
            { fuel=fuel, size=size } )
      if nocol then
         return s
      end
      if hilight then
         return "#b"..s.."#0"
      else
         return "#n"..s.."#0"
      end
   end

   local size = (p and p:size()) or -1
   local str = addstr( FUEL_LARGE, _("large"), size>4, size<0 ).."\n"
   str = str..addstr( FUEL_MEDIUM, _("medium"), (size>2) and (size<=4), size<0 ).."\n"
   return str..addstr( FUEL_SMALL, _("small"), (size>0) and (size<=2), size<0 )
end

function init( p, po )
   po:set( "fuel", fuel_from_size(p) )
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
