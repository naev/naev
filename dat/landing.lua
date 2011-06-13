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

-- Default function. Any asset that has no landing script explicitly defined will use this.
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
      bribe_price = -standing * 1000 -- Shouldn't be random as this can be recalculated many times.
      local str   = numstring( bribe_price )
      bribe_msg   = string.format(
            "\"I'll let you land for the modest price of %s credits.\"\n\nPay %s credits?",
            str, str )
      bribe_ack_msg = "Make it quick."
   end
   return can_land, land_msg, bribe_price, bribe_msg, bribe_ack_msg
end

-- Empire military assets.
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

-- Empire Polaris Prime.
function emp_mil_polprime( pnt )
   local fct = pnt:faction()
   local standing = fct:playerStanding()
   local can_land = standing > 70

   local land_msg
   if can_land then
      land_msg = "The Emperor permits you to land."
   elseif standing >= 0 then
      land_msg = "You may not approach the Emperor."
   else
      land_msg = "Landing request denied."
   end

   return can_land, land_msg
end

-- Sirius military assets.
function srs_mil_restricted( pnt )
   local fct = pnt:faction()
   local standing = fct:playerStanding()
   local can_land = standing > 30

   local land_msg
   if can_land then
      land_msg = "Permission to land granted."
   elseif standing >= 0 then
      land_msg = "Only the faithful may land here. Request denied."
   else
      land_msg = "Landing request denied."
   end

   return can_land, land_msg
end

-- Sirius Mutris.
function srs_mil_mutris( pnt )
   local fct = pnt:faction()
   local standing = fct:playerStanding()
   local can_land = standing > 50

   local land_msg
   if can_land then
      land_msg = "Welcome to Mutris, home of Sirichana."
   elseif standing >= 0 then
      land_msg = "You may not approach the home of Sirichana yet."
   else
      land_msg = "Landing request denied."
   end

   return can_land, land_msg
end

-- Dvaered military assets.
function dv_mil_restricted( pnt )
   local fct = pnt:faction()
   local standing = fct:playerStanding()
   local can_land = standing > 40

   local land_msg
   if can_land then
      land_msg = "Permission to land granted."
   elseif standing >= 0 then
      land_msg = "Your rank is too low, citizen. Access denied."
   else
      land_msg = "Landing request denied."
   end

   return can_land, land_msg
end

-- Dvaered High Command.
function dv_mil_restricted( pnt )
   local fct = pnt:faction()
   local standing = fct:playerStanding()
   local can_land = standing > 70

   local land_msg
   if can_land then
      land_msg = "Permission to land granted, sir."
   elseif standing >= 0 then
      land_msg = "Only high ranking personnel allowed. Landing request denied."
   else
      land_msg = "Landing request denied."
   end

   return can_land, land_msg
end

-- Pirate clanworld.
function pir_clanworld( pnt )
   local fct = pnt:faction()
   local standing = fct:playerStanding()
   local can_land = standing > 20

   local land_msg
   if can_land then
      land_msg = "Permission to land granted. Welcome, brother."
   elseif standing >= 0 then
      land_msg = "Small time pirates have no business on our clanworld!"
   else
      land_msg = "Get out of here!"
   end

   return can_land, land_msg
end







