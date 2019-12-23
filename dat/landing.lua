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

include "dat/scripts/numstring.lua"

-- Default function. Any asset that has no landing script explicitly defined will use this.
function land( pnt )
   return land_civilian(pnt, 0, -30)
end

-- Low-class landing function. Low class planets let you land and bribe at much lower standings.
function land_lowclass( pnt )
   return land_civilian(pnt, -20, -80)
end

-- High class landing function. High class planets can't be bribed.
function land_hiclass( pnt )
   return land_civilian(pnt, 0, 0)
end

-- Empire military assets.
function emp_mil_restricted( pnt )
   return land_military(pnt, 35,
         _("Permission to land granted."),
         _("You are not authorized to land here."),
         _("Landing request denied."),
         _("\"Don't attempt to bribe an Empire official, pilot.\""))
end

-- Empire Omega Station.
function emp_mil_omega( pnt )
   local required = 35

   if player.misnDone("Empire Shipping 3") or player.misnActive("Empire Shipping 3") then
      required = 0
   end

   return land_military(pnt, required,
         _("Permission to land granted."),
         _("You are not authorized to land here."),
         _("Landing request denied."),
         _("\"Don't attempt to bribe an Empire official, pilot.\""))
end

-- Empire Emperor's Wrath.
function emp_mil_wrath( pnt )
   return land_military(pnt, 75,
         _("The Emperor permits you to land."),
         _("You may not approach the Emperor."),
         _("Landing request denied."),
         _("\"Don't attempt to bribe an Empire official, pilot.\""))
end

-- Sirius military assets.
function srs_mil_restricted( pnt )
   return land_military(pnt, 30,
         _("Permission to land granted."),
         _("Only the faithful may land here. Request denied."),
         _("Landing request denied."),
         _("\"The faithful will never be swayed by money.\""))
end

-- Sirius Mutris.
function srs_mil_mutris( pnt )
   return land_military(pnt, 75,
         _("Welcome to Mutris, home of Sirichana."),
         _("You may not approach the home of Sirichana yet."),
         _("Landing request denied."),
         _("\"The faithful will never be swayed by money.\""))
end

-- Dvaered military assets.
function dv_mil_restricted( pnt )
   return land_military(pnt, 40,
         _("Permission to land granted."),
         _("Your rank is too low, citizen. Access denied."),
         _("Landing request denied."),
         _("\"Money won't buy you access to our restricted facilities, citizen.\""))
end

-- Dvaered High Command.
function dv_mil_command( pnt )
   return land_military(pnt, 80,
         _("Permission to land granted, captain."),
         _("Only high ranking personnel allowed. Landing request denied."),
         _("Landing request denied."),
         _("\"Money won't buy you access to our restricted facilities, citizen.\""))
end

-- Soromid military assets.
function srm_mil_restricted( pnt )
   return land_military(pnt, 30,
         _("Permission to land granted."),
         _("Permission denied. You're not truly one of us."),
         _("Landing request denied."),
         _("\"We don't need your money, outsider.\""))
end

-- Soromid Kataka.
function srm_mil_kataka( pnt )
   return land_military(pnt, 75,
         _("Permission to land granted."),
         _("Only friends of the Soromid may set foot on Kataka."),
         _("Landing request denied."),
         _("\"We don't need your money, outsider.\""))
end


-- Za'lek's military assets.
function zlk_mil_restricted( pnt )
   return land_military(pnt, 30,
         _("Docking sequence transmitted."),
         _("Authorization level too low to grant access."),
         _("Authorization denied."),
         _("Money is irrelevant."))
end


-- Za'lek's military center.
function zlk_ruadan( pnt )
   return land_military(pnt, 75,
         _("Docking sequence transmitted."),
         _("This is a restricted area. Your clearance is far too low. Go away."),
         _("Authorization denied."),
         _("Bribery is a crime, and will not get you on this planet."))
end

-- Pirate clanworld.
function pir_clanworld( pnt )
   local fct = pnt:faction()
   local standing = fct:playerStanding()
   local can_land = standing > 20 or pnt:getLandOverride()

   local land_msg
   if can_land then
      land_msg = _("Permission to land granted. Welcome, mate.")
   elseif standing >= 0 then
      land_msg = _("Small time pirates have no business on our clanworld!")
   else
      land_msg = _("Get out of here!")
   end

   -- Calculate bribe price. Custom for pirates.
   local bribe_price, bribe_msg, bribe_ack_msg
   if not can_land and standing >= -50 then
      bribe_price = (20 - standing) * 500 + 1000 -- 36K max, at -50 rep. Pirates are supposed to be cheaper than regular factions.
      local str   = numstring( bribe_price )
      bribe_msg   = string.format(
            _("\"Well, I think you're scum, but I'm willing to look the other way for %s credits. Deal?\""),
            str )
      bribe_ack_msg = _("Heh heh, thanks. Now get off the comm, I'm busy!")
   end
   return can_land, land_msg, bribe_price, bribe_msg, bribe_ack_msg
end

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
-- Expects the faction, the minimum standing to land, the minimum standing to bribe, and a going rate for bribes.
-- Returns whether the planet can be bribed, and the cost for doing so.
function getcost(fct, land_floor, bribe_floor, rate)
   local standing = fct:playerStanding()
   if standing < bribe_floor then
      return _("\"I'm not dealing with dangerous criminals like you!\"")
   else
      -- Assume standing is always lower than the land_floor.
      return (land_floor - standing) * rate * getshipmod() + 5000
   end
end

-- Civilian planet landing logic.
-- Expects the planet, the lowest standing at which landing is allowed, and the lowest standing at which bribing is allowed.
function land_civilian( pnt, land_floor, bribe_floor )
   local fct = pnt:faction()
   local can_land = fct:playerStanding() >= land_floor or pnt:getLandOverride()

   -- Get land message
   local land_msg
   if can_land then
      land_msg = _("Permission to land granted.")
   else
      land_msg = _("Landing request denied.")
   end

   local bribe_msg, bribe_ack_msg
   -- Calculate bribe price. Note: Assumes bribe floor < land_floor.
   local bribe_price = getcost(fct, land_floor, bribe_floor, 1000) -- TODO: different rates for different factions.
   if not can_land and type(bribe_price) == "number" then
       local str      = numstring( bribe_price )
       bribe_msg      = string.format(_("\"I'll let you land for the modest price of %s credits.\"\n\nPay %s credits?"), str, str )
       bribe_ack_msg  = _("Make it quick.")
   end
   return can_land, land_msg, bribe_price, bribe_msg, bribe_ack_msg
end

-- Military planet landing logic.
-- Expects the planet, the lowest standing at which landing is allowed, and four strings:
-- Landing granted string, standing too low string, landing denied string, message upon bribe attempt.
function land_military( pnt, land_floor, ok_msg, notyet_msg, no_msg, nobribe )
   local fct = pnt:faction()
   local standing = fct:playerStanding()
   local can_land = standing >= land_floor or pnt:getLandOverride()

   local land_msg
   if can_land then
      land_msg = ok_msg
   elseif standing >= 0 then
      land_msg = notyet_msg
   else
      land_msg = no_msg
   end

   return can_land, land_msg, nobribe
end
