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
   local standing = fct:playerStanding()
   local can_land = standing >= 0

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
      bribe_price = math.abs(standing) * 1000 -- Shouldn't be random as this can be recalculated many times.
      local str   = numstring( bribe_price )
      bribe_msg   = string.format(
            "\"I'll let you land for the modest price of %s credits.\"\n\nPay %s credits?",
            str, str )
      bribe_ack_msg = "Make it quick."
   end
   return can_land, land_msg, bribe_price, bribe_msg, bribe_ack_msg
end

function land_getMilitaryMessages( fname, can_land, standing )
   local message, land_msg, messages = {}
   local nobribe = {
      Empire  = "\"Don't attempt to bribe an Empire official, pilot.\"",
      Dvaered = "\"Money won't buy you access to our restricted facilities, citizen.\"",
      Sirius  = "\"The faithful will never be swayed by money.\""
   }
   if can_land then
      if fname == "Pirate" then
         land_msg = "Permission to land granted. Welcome, brother."
      else
         land_msg = "Permission to land granted."
      end
   elseif standing >= 0 then
      messages = {
         Empire  = "You are not authorized to land here.",
         Dvaered = "Your rank is too low, citizen. Access denied.",
         Sirius  = "Only the faithful may land here. Request denied.",
         Pirate = "Small time pirates have no business on our clanworld!"
      }
   else -- Hostile.
      if fname == "Pirate" then
         land_msg = "Get out of here!"
      else
         land_msg = "Landing request denied."
      end
   end
   return land_msg or messages[fname], nobribe[fname]
end

function military( pnt )
   local fct = pnt:faction()
   local fname = fct:name()
   local standing = fct:playerStanding()
   local req = { Dvaered = 40, Pirate  = 20 }
   local can_land, nobribe, land_msg

   if req[fname] then
      can_land = standing > req[fname]
   else -- Default.
      can_land = standing > 30
   end

   land_msg, nobribe = land_getMilitaryMessages( fname, can_land, standing )
   local can_bribe, bribe_price, bribe_msg, bribe_ack_msg
   if not can_land and fname == "Pirate" then
      bribe_price = (40 + math.abs(standing)) * 3000 -- Shouldn't be random as this can be recalculated many times.
      local str   = numstring( bribe_price )
      bribe_msg   = string.format(
            "\"Well, I think you're scum, but I'm willing to look the other way for %s credits. Deal?\"",
            str )
      bribe_ack_msg = "Heh heh, thanks. Now get off the comm, I'm busy!"
   end

   return can_land, land_msg, bribe_price or nobribe, bribe_msg, bribe_ack_msg
end

function land_getSpecialMessages( pname, can_land, standing )
   local message, land_msg, messages = {}
   local nobribe = {
      ["Polaris Prime"] = "\"Don't attempt to bribe an Empire official, pilot.\"",
      ["Dvaered High Command"] = "\"Money won't buy you access to our restricted facilities, citizen.\"",
      ["Mutris"] = "\"The faithful will never be swayed by money.\""
   }
   if can_land then
      messages = {
         ["Polaris Prime"] = "The Emperor permits you to land.",
         ["Dvaered High Command"] = "Permission to land granted, sir.",
         ["Mutris"]  = "Welcome to Mutris, home of Sirichana."
      }
   elseif standing >= 0 then
      messages = {
         ["Polaris Prime"] = "You may not approach the Emperor.",
         ["Dvaered High Command"] = "Only high ranking personnel allowed. Landing request denied.",
         ["Mutris"] = "You may not approach the home of Sirichana yet."
      }
   else -- Hostile.
      land_msg = "Landing request denied."
   end
   return land_msg or messages[pname], nobribe[pname]
end

function special( pnt )
   local fct = pnt:faction()
   local fname = fct:name()
   local standing = fct:playerStanding()
   local req = { Sirius  = 50 }
   local can_land, land_msg, nobribe

   if req[fname] then
      can_land = standing > req[fname]
   else -- Default.
      can_land = standing > 70
   end

   land_msg, nobribe = land_getSpecialMessages( pnt:name(), can_land, standing )
   return can_land, land_msg, nobribe
end
