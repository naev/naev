include("dat/factions/equip/generic.lua")


equip_shipOutfits_coreSystems["Dvaered Vendetta"] = {
   "Milspec Hermes 3602 Core System"
}
equip_shipOutfits_coreSystems["Dvaered Ancestor"] = {
   "Milspec Hermes 3602 Core System"
}
equip_shipOutfits_coreSystems["Dvaered Phalanx"] = {
   "Milspec Hermes 4702 Core System"
}
equip_shipOutfits_coreSystems["Dvaered Vigilance"] = {
   "Milspec Hermes 5402 Core System"
}
equip_shipOutfits_coreSystems["Dvaered Goddard"] = {
   "Milspec Hermes 9802 Core System"
}

equip_shipOutfits_weapons["Dvaered Vendetta"] = {
   {
      num = 2, varied = true;
      "Vulcan Gun", "Shredder", "Unicorp Mace Launcher"
   },
   {
      "Vulcan Gun", "Shredder"
   }
   
}
equip_shipOutfits_weapons["Dvaered Ancestor"] = {
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
equip_shipOutfits_weapons["Dvaered Phalanx"] = {
   {
      num = 3;
      "Mass Driver MK2", "Mass Driver MK3"
   },
   {
      varied = true;
      "Shredder", "Turreted Gauss Gun", "Unicorp Mace Launcher"
   }
}
equip_shipOutfits_weapons["Dvaered Vigilance"] = {
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
equip_shipOutfits_weapons["Dvaered Goddard"] = {
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
