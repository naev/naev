local equipopt = require 'equipopt'
local mt = require 'merge_tables'

--[[
equip_typeOutfits_hulls["Vendetta"] = {
   "S&K Light Combat Plating",
}
equip_typeOutfits_hulls["Ancestor"] = {
   "S&K Light Combat Plating",
}
equip_typeOutfits_hulls["Phalanx"] = {
   "S&K Medium Combat Plating",
}
equip_typeOutfits_hulls["Vigilance"] = {
   "S&K Medium-Heavy Combat Plating",
}
equip_typeOutfits_hulls["Goddard"] = {
   "S&K Superheavy Combat Plating",
}

equip_typeOutfits_weapons["Vendetta"] = {
   {
      num = 2, probability = {
         ["Vulcan Gun"] = 4, ["Gauss Gun"] = 4,
      };
      "Vulcan Gun", "Gauss Gun",
      "Unicorp Mace Launcher", "TeraCom Mace Launcher",
   },
   {
      num = 2, probability = {
         ["Vulcan Gun"] = 4, ["Gauss Gun"] = 4,
      };
      "Vulcan Gun", "Gauss Gun",
      "Unicorp Mace Launcher", "TeraCom Mace Launcher",
   },
   {
      probability = {
         ["Vulcan Gun"] = 4, ["Gauss Gun"] = 4,
      };
      "Vulcan Gun", "Gauss Gun",
      "Unicorp Mace Launcher", "TeraCom Mace Launcher",
   },
}
equip_typeOutfits_weapons["Ancestor"] = {
   {
      num = 2, varied = true;
      "Unicorp Headhunter Launcher", "Unicorp Vengeance Launcher",
   },
   {
      varied = true;
      "Vulcan Gun", "TeraCom Mace Launcher",
   },
}
equip_typeOutfits_weapons["Phalanx"] = {
   {
      num = 3, varied = true;
      "Unicorp Headhunter Launcher", "Unicorp Vengeance Launcher",
      "TeraCom Headhunter Launcher", "TeraCom Vengeance Launcher",
      "Unicorp Caesar IV Launcher", "TeraCom Imperator Launcher",
   },
   {
      probability = {
         ["Shredder"] = 8,
      };
      "Shredder", "Vulcan Gun",
   },
}
equip_typeOutfits_weapons["Vigilance"] = {
   {
      num = 2;
      "Railgun",
   },
   {
      num = 1;
      "Enygma Systems Turreted Fury Launcher",
      "Enygma Systems Turreted Headhunter Launcher",
   },
   {
      num = 1;
      "Mass Driver", "Turreted Vulcan Gun",
      "Enygma Systems Turreted Fury Launcher",
      "Enygma Systems Turreted Headhunter Launcher",
   },
   {
      "Mass Driver", "Turreted Vulcan Gun",
   },
}
equip_typeOutfits_weapons["Goddard"] = {
   {
      num = 1;
      "Enygma Systems Turreted Fury Launcher",
      "Enygma Systems Turreted Headhunter Launcher",
   },
   {
      "Repeating Railgun",
   },
   {
      "Railgun", "Heavy Laser Turret", "Grave Beam", "Heavy Ion Turret",
   },
}

equip_typeOutfits_structurals["Phalanx"] = {
   {
      varied = true;
      "Medium Fuel Pod", "Battery II", "Shield Capacitor II",
      "Plasteel Plating", "Active Plating",
   },
}
equip_typeOutfits_structurals["Vigilance"] = {
   {
      varied = true;
      "Large Fuel Pod", "Battery III", "Shield Capacitor IV",
      "Shield Capacitor III", "Nanobond Plating",
   },
   {
      varied = true;
      "Medium Fuel Pod", "Battery II", "Shield Capacitor II",
      "Plasteel Plating", "Active Plating",
   },
}


local dva_util_small = { { "MilSpec Impacto-Plastic Coating" } }
local dva_util_medplus = { { varied = true;
   "Cyclic Combat AI", "MilSpec Impacto-Plastic Coating" } }
equip_shipOutfits_utilities["Dvaered Vendetta"] = dva_util_small
equip_shipOutfits_utilities["Dvaered Ancestor"] = dva_util_small
equip_shipOutfits_utilities["Dvaered Phalanx"]  = dva_util_medplus
equip_shipOutfits_utilities["Dvaered Vigilance"]= dva_util_medplus
equip_shipOutfits_utilities["Dvaered Goddard"]  = dva_util_medplus
--]]

local dvaered_outfits = {
   -- Heavy Weapons
   "Railgun", "Repeating Railgun",
   "Heavy Laser Turret", "Grave Beam", "Heavy Ion Turret",
   -- Medium Weapons
   "Mass Driver", "Turreted Vulcan Gun",
   "Enygma Systems Turreted Fury Launcher",
   "Enygma Systems Turreted Headhunter Launcher",
   "Unicorp Headhunter Launcher", "Unicorp Vengeance Launcher",
   "TeraCom Headhunter Launcher", "TeraCom Vengeance Launcher",
   "Unicorp Caesar IV Launcher", "TeraCom Imperator Launcher",
   -- Small Weapons
   "Shredder",
   "Vulcan Gun", "Gauss Gun",
   "Unicorp Mace Launcher", "TeraCom Mace Launcher",
   -- Utility
   "Cyclic Combat AI", "MilSpec Impacto-Plastic Coating",
   "Unicorp Scrambler", "Unicorp Light Afterburner",
   "Sensor Array", "Hellburner", "Emergency Shield Booster",
   "Unicorp Medium Afterburner", "Droid Repair Crew",
   -- Heavy Structural
   "Large Fuel Pod", "Battery III", "Reactor Class III",
   "Shield Capacitor III", "Nanobond Plating", "Biometal Armour",
   -- Medium Structural
   "Medium Fuel Pod", "Battery II", "Shield Capacitor II",
   "Active Plating", "Reactor Class II",
   -- Small Structural
   "Plasteel Plating", "Battery I", "Improved Stabilizer", "Engine Reroute",
   "Reactor Class I", "Small Fuel Pod",
}


local dvaered_params = {
}
local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end
local dvaered_cores = {
   ["Dvaered Vendetta"] = function () return {
         "S&K Light Combat Plating",
      } end,
   ["Dvaered Ancestor"] = function () return {
         "S&K Light Combat Plating",
      } end,
   ["Dvaered Phalanx"] = function () return {
         "S&K Medium Combat Plating",
      } end,
   ["Dvaered Vigilance"] = function () return {
         "S&K Medium-Heavy Combat Plating",
      } end,
   ["Dvaered Goddard"] = function () return {
         "S&K Superheavy Combat Plating",
      } end,
}

--[[
-- @brief Does Dvaered pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   local sname = p:ship():nameRaw()
   --if dvaered_skip[sname] then return end

   -- Choose parameters and make Za'lekish
   local params = equipopt.params.choose( p )
   -- Prefer to use the Za'lek utilities
   params.prefer["Cyclic Combat AI"] = 100
   params.prefer["MilSpec Impacto-Plastic Coating"] = 100
   params.max_same_stru = 3
   params.max_mass = 0.95 + 0.2*rnd.rnd()
   -- Per ship tweaks
   local sp = dvaered_params[ sname ]
   if sp then
      params = mt.merge_tables_recursive( params, sp() )
   end

   -- See cores
   local cores = dvaered_cores[ sname ]()

   -- Try to equip
   equipopt.equip( p, cores, dvaered_outfits, params )
end
