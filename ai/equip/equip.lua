
-- Outfit definitions
include("ai/equip/outfits.lua")

-- Helper functions
include("ai/equip/helper.lua")

-- Equipping algorithms
include("ai/equip/generic.lua")
include("ai/equip/pirate.lua")
include("ai/equip/empire.lua")
include("ai/equip/dvaered.lua")
include("ai/equip/sirius.lua")


--[[
-- Some faction definitions for faster lookups.
--]]
_eq_emp = faction.get("Empire")
_eq_god = faction.get("Goddard")
_eq_pro = faction.get("Proteron")
_eq_dva = faction.get("Dvaered")
_eq_pir = faction.get("Pirate")
_eq_sir = faction.get("Sirius")


--[[
-- @brief Equips a pilot
--
--    @param p Pilot to equip
--    @param f Faction to which pilot belongs
--]]
function equip ( p, f )
   if f == _eq_emp or f == _eq_god or f == _eq_pro then
      equip_empire( p )
   elseif f == _eq_dva then
      equip_dvaered( p )
   elseif f == _eq_sir then
      equip_sirius( p )
   elseif f == _eq_pir then
      equip_pirate( p )
   else
      equip_generic( p )
   end
end

