--[[
   The way these work is they must return two parameters, the first is a
    boolean indicating whether or not permission has been granted. The
    second is an optional string of the message to display. So something
    like:

   function noland( pnt )
      return false, "Nobody expects the spanish inquisition!"
   end

--]]

include "scripts/numstring.lua"


function land( pnt )
   local fct = pnt:faction()
   local can_land = fct:playerStanding() > 0

   -- Get land message
   local land_msg
   if can_land then
      land_msg = "Permission to land granted."
   else
      land_msg = "Landing request denied."
   end

   -- Calculate bribe price
   local can_bribe, bribe_price, bribe_msg, bribe_ack_msg
   if not can_land then
      bribe_price = 10000 -- Shouldn't be random as this can be recalculated many times
      local str   = numstring( bribe_price )
      bribe_msg   = string.format(
            "\"I'll let you land for the modest price of %s credits.\"\n\nPay %s credits?",
            str, str )
      bribe_ack_msg = "Make it quick."
   end
   return can_land, land_msg, bribe_price, bribe_msg, bribe_ack_msg
end


function emp_mil_restricted( pnt )
   local fct = pnt:faction()
   local standing = fct:playerStanding()
   local can_land = standing > 30

   local land_msg
   if can_land then
      land_msg = "Permission to land granted."
   elseif standing >= 0 then
      land_msg = "You are not authorized to land here."
   else
      land_msg = "Landing request denied."
   end

   return can_land, land_msg
end





