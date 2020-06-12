include("dat/factions/equip/generic.lua")


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


--[[
-- @brief Does soromid pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   equip_generic( p )
end
