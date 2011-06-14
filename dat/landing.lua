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

-- Helper function for determining the bribe cost multiplier for the player's current ship.
-- Returns the factor the bribe cost is multiplied by when the player tries to bribe.
-- NOTE: This should be replaced by something better in time.
function getshipmod()
    local light = {"Yacht", "Luxury Yacht", "Drone", "Fighter", "Bomber", "Scout"}
    local medium = {"Destroyer", "Corvette", "Courier", "Armoured Transport", "Freighter"}
    local heavy = {"Cruiser", "Carrier"}
    local ps = player.pilot():ship()
    for _, j in ipairs(light) do
        if ps == j then return 1 end
    end
    for _, j in ipairs(medium) do
        if ps == j then return 2 end
    end
    for _, j in ipairs(heavy) do
        if ps == j then return 4 end
    end
    return 1
end

-- Helper function for calculating bribe availability and cost.
-- Expects the faction, a floor value for standing and a going rate for bribes.
-- Returns whether the planet can be bribed, and the cost for doing so.
function getcost(fct, floor, rate)
    local standing = fct:playerStanding()
    if standing < floor then
        return "I'm not dealing with dangerous criminals like you!"
    else
        -- Assume standing is always negative.
        return -standing * rate * getshipmod() + 5000
    end
end

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

   local bribe_msg, bribe_ack_msg
   -- Calculate bribe price
   local bribe_price = getcost(fct, -30, 1000) -- TODO: different rates for different factions.
   if not can_land and type(bribe_price) == "number" then
       local str            = numstring( bribe_price )
       bribe_msg      = string.format("\"I'll let you land for the modest price of %s credits.\"\n\nPay %s credits?", str, str )
       bribe_ack_msg  = "Make it quick."
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

   -- Calculate bribe price. Custom for pirates.
   local can_bribe, bribe_price, bribe_msg, bribe_ack_msg
   if not can_land and standing >= -50 then
      bribe_price = (20 - standing) * 500 + 1000 -- 36K max, at -50 rep. Pirates are supposed to be cheaper than regular factions.
      local str   = numstring( bribe_price )
      bribe_msg   = string.format(
            "\"Well, I think you're scum, but I'm willing to look the other way for %s credits. Deal?\"",
            str )
      bribe_ack_msg = "Heh heh, thanks. Now get off the comm, I'm busy!"
   end
   return can_land, land_msg, bribe_price, bribe_msg, bribe_ack_msg
end
