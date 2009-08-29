
-- Outfit definitions
include("ai/equip/outfits.lua")

-- Helper functions
include("ai/equip/helper.lua")

-- Equipping algorithms
include("ai/equip/generic.lua")
include("ai/equip/pirate.lua")
include("ai/equip/empire.lua")
include("ai/equip/dvaered.lua")


--[[
-- @brief Equips a pilot
--
--    @param p Pilot to equip
--    @param f Faction to which pilot belongs
--]]
function equip ( p, f )
   if f == faction.get( "Empire" ) or f == faction.get( "Goddard" ) then
      equip_empire( p )
   elseif f == faction.get( "Dvaered" ) then
      equip_dvaered( p )
   elseif f == faction.get( "Pirate" ) then
      equip_pirate( p )
   else
      equip_generic( p )
   end
end

