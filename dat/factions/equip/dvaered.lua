require("factions/equip/generic.lua")


equip_typeOutfits_coreSystems["Vendetta"] = {
   "Milspec Hermes 3602 Core System"
}
equip_typeOutfits_coreSystems["Ancestor"] = equip_typeOutfits_coreSystems["Vendetta"]
equip_typeOutfits_coreSystems["Phalanx"] = {
   "Milspec Hermes 4702 Core System"
}
equip_typeOutfits_coreSystems["Vigilance"] = {
   "Milspec Hermes 5402 Core System"
}
equip_typeOutfits_coreSystems["Goddard"] = {
   "Milspec Hermes 9802 Core System"
}

equip_typeOutfits_weapons["Vendetta"] = {
   {
      num = 2, varied = true;
      "Vulcan Gun", "Shredder", "Unicorp Mace Launcher"
   },
   {
      "Vulcan Gun", "Shredder"
   }
   
}
equip_typeOutfits_weapons["Ancestor"] = {
   {
      num = 2;
      "Unicorp Mace Launcher"
   },
   {
      num = 1;
      "Unicorp Mace Launcher", "Vulcan Gun", "Shredder"
   },
   {
      "Vulcan Gun", "Shredder"
   }
   
}
equip_typeOutfits_weapons["Phalanx"] = {
   {
      num = 3;
      "Mass Driver MK2", "Mass Driver MK3"
   },
   {
      varied = true;
      "Shredder", "Turreted Gauss Gun", "Unicorp Mace Launcher"
   }
}
equip_typeOutfits_weapons["Vigilance"] = {
   {
      num = 2;
      "Railgun", "Repeating Railgun"
   },
   {
      varied = true;
      "Turreted Vulcan Gun", "Mass Driver MK2", "Mass Driver MK3",
      "Unicorp Mace Launcher"
   }
}
equip_typeOutfits_weapons["Goddard"] = {
   {
      varied = true;
      "Railgun Turret", "Repeating Railgun", "Mass Driver MK3",
      "Turreted Vulcan Gun", "Unicorp Mace Launcher"
   }
}



--[[
-- @brief Does dvaered pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   equip_generic( p )
end
