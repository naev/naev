require "factions/equip/generic.lua"


equip_typeOutfits_weapons["Brigand"] = {
   {
      varied = true, probability = {
         ["BioPlasma Organ Stage X"] = 3
      };
      "BioPlasma Organ Stage 1", "BioPlasma Organ Stage 2",
      "BioPlasma Organ Stage 3", "BioPlasma Organ Stage X",
      "Plasma Blaster MK3"
   }
}

-- TODO: Design bio-weapons for larger ship types, then equip them here.


--[[
-- @brief Does Soromid pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   equip_generic( p )
end
