
--[[
-- Pirate Fame/Faction Standing script

   When the player enters a system, his fame has a chance of being lowered.

   If he is using a pirate ship, the chances of his fame being lowered are
   reduced. If the player is using an impressive non-pirate ship, like the
   cruiser or carrier of a major faction, will lower a bit less often, but
   will lower more often than if he was using a purely pirate ship.

   This event will not reduce the player’s fame below a given level.

--]]

local function has(i,t)
   for n = 1,#t do
      if t[n] == i then
         return true
      end
   end
   return false
end

--[[
-- Returns a boolean indicating whether or not the player is using a pirate
-- ship.
--]]
local function using_pirate_ship()
   local s = player.pilot():ship():name()

   return has(s, {
      "Hyena",
      "Pirate Kestrel",
      "Pirate Admonisher",
      "Pirate Phalanx",
      "Pirate Ancestor",
      "Pirate Vendetta",
      "Pirate Shark",
      "Pirate Rhino"
   })
end

--[[
-- Returns a boolean indicating whether or not the player is using some kind
-- of monstruously powerfull or intimidating ship, like another’s faction
-- cruiser or carrier.
--]]
local function using_impressive_ship()
   local s = player.pilot():ship():name()

   return has(s, {
      "Empire Peacemaker",
      "Empire Hawking",
      "Sirius Divinity",
      "Sirius Dogma",
      "Soromid Arx",
      "Soromid Ira",
      "Dvaered Goddard",
      -- Still impressive, but purely “civilian”
      "Goddard",
      "Hawking",
      "Kestrel"
   })
end

function create()
   local fame = faction.playerStanding("Pirate")

   local floor = var.peek("_ffloor_decay_pirate")
   if floor == nil then floor = -20 end
   if fame <= floor then
      evt.finish()
   end

   local amt
   if using_pirate_ship() then
      amt = 0.15 + rnd.sigma() * 0.05
   elseif using_impressive_ship() then
      amt = 0.5 + rnd.sigma() * 0.10
   else
      amt = 1 + rnd.sigma() * 0.25
   end

   faction.modPlayerSingle( "Pirate", -amt )

   evt.finish()
end

