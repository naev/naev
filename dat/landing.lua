--[[
   Prototype function:

      Parameter: pnt - Planet to set landing stuff about.
      Return: 1) Boolean whether or not can land
              2) Land message which should be denial if can't land or acceptance if can
              3) (optional) Bribe price or message that can't be bribed or nil in the case of no possibility to bribe.
              4) (Needed with bribe price) Bribe message telling the price to pay
              5) (Needed with bribe price) Bribe acceptance message

   Examples:

   function yesland( pnt )
      return true, "Come on in dude"
   end

   function noland( pnt )
      return false, "Nobody expects the spanish inquisition!"
   end

   function noland_nobribe( pnt )
      return false, "No can do.", "Seriously, don't even think of bribing me dude."
   end

   function noland_yesbribe( pnt )
      return false, "You can't land buddy", 500, "But you can bribe for 500 credits", "Thanks for the money"
   end

--]]

include "scripts/numstring.lua"

-- Default function. Any asset that has no landing script explicitly defined will use this.
function land( pnt )
   local fct = pnt:faction()
   local can_land = fct:playerStanding() >= 0

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
      bribe_price = math.max(-standing, 5) * 1000 -- Shouldn't be random as this can be recalculated many times.
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

   local nobribe = "\"Don't attempt to bribe an Empire official, pilot.\""

   return can_land, land_msg, nobribe
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

   local nobribe = "\"Don't attempt to bribe an Empire official, pilot.\""

   return can_land, land_msg, nobribe
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

   local nobribe = "\"The faithful will never be swayed by money.\""

   return can_land, land_msg, nobribe
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

   local nobribe = "\"The faithful will never be swayed by money.\""

   return can_land, land_msg, nobribe
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

   local nobribe = "\"Money won't buy you access to our restricted facilities, citizen.\""

   return can_land, land_msg, nobribe
end

-- Dvaered High Command.
function dv_mil_command( pnt )
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

   local nobribe = "\"Money won't buy you access to our restricted facilities, citizen.\""

   return can_land, land_msg, nobribe
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

   -- Calculate bribe price
   local can_bribe, bribe_price, bribe_msg, bribe_ack_msg
   if not can_land then
      bribe_price = math.max((40 - standing), 5) * 3000 -- Shouldn't be random as this can be recalculated many times.
      local str   = numstring( bribe_price )
      bribe_msg   = string.format(
            "\"Well, I think you're scum, but I'm willing to look the other way for %s credits. Deal?\"",
            str )
      bribe_ack_msg = "Heh heh, thanks. Now get off the comm, I'm busy!"
   end
   return can_land, land_msg, bribe_price, bribe_msg, bribe_ack_msg
end
