--[[--
Functions for handling portraits in Naev.

Note that this is mainly a transitional package and is deprecated. Use vnimage when possible instead.

@module portrait
--]]
--[[

   Portrait Common

--]]
local portrait = {}

local portraits_m = {}
local portraits_f = {}
local portraits_mil_m = {}
local portraits_mil_f = {}

local portraits_m_neutral = {
   "neutral/male1.webp",
   "neutral/male2.webp",
   "neutral/male3.webp",
   "neutral/male4.webp",
   "neutral/male5.webp",
   "neutral/male6.webp",
   "neutral/male7.webp",
   "neutral/male8.webp",
   "neutral/male9.webp",
   "neutral/male10.webp",
   "neutral/male11.webp",
   "neutral/male12.webp",
   "neutral/male13.webp",
   "neutral/miner1.webp",
   "neutral/thief1.webp",
   "neutral/thief2.webp",
}
local portraits_f_neutral = {
   "neutral/female1.webp",
   "neutral/female2.webp",
   "neutral/female3.webp",
   "neutral/female4.webp",
   "neutral/female5.webp",
   "neutral/female6.webp",
   "neutral/female7.webp",
   "neutral/female8.webp",
   "neutral/female9.webp",
   "neutral/female10.webp",
   "neutral/female11.webp",
   "neutral/female12.webp",
   "neutral/female13.webp",
   "neutral/miner2.webp",
   "neutral/thief3.webp",
   "neutral/thief4.webp",
}

portraits_m["Dvaered"] = {
   "dvaered/dv_civilian_m1.webp",
   "dvaered/dv_civilian_m2.webp",
   "dvaered/dv_civilian_m3.webp",
   "dvaered/dv_civilian_m4.webp",
   "dvaered/dv_civilian_m5.webp",
   "dvaered/dv_civilian_m6.webp",
   "dvaered/dv_civilian_m7.webp",
   "dvaered/dv_civilian_m8.webp",
   "dvaered/dv_civilian_m9.webp",
   "dvaered/dv_civilian_m10.webp",
   "dvaered/dv_civilian_m11.webp",
}
portraits_f["Dvaered"] = {
   "dvaered/dv_civilian_f1.webp",
   "dvaered/dv_civilian_f2.webp",
   "dvaered/dv_civilian_f3.webp",
   "dvaered/dv_civilian_f4.webp",
   "dvaered/dv_civilian_f5.webp",
   "dvaered/dv_civilian_f6.webp",
   "dvaered/dv_civilian_f7.webp",
   "dvaered/dv_civilian_f8.webp",
   "dvaered/dv_civilian_f9.webp",
   "dvaered/dv_civilian_f10.webp",
   "dvaered/dv_civilian_f11.webp",
}

portraits_m["Sirius"] = {
   "sirius/sirius_fyrra_m1.webp",
   "sirius/sirius_fyrra_m2.webp",
   "sirius/sirius_fyrra_m3.webp",
   "sirius/sirius_fyrra_m4.webp",
   "sirius/sirius_fyrra_m5.webp",
   "sirius/sirius_shaira_m1.webp",
   "sirius/sirius_shaira_m2.webp",
   "sirius/sirius_shaira_m3.webp",
   "sirius/sirius_shaira_m4.webp",
   "sirius/sirius_shaira_m5.webp",
}
portraits_f["Sirius"] = {
   "sirius/sirius_fyrra_f1.webp",
   "sirius/sirius_fyrra_f2.webp",
   "sirius/sirius_fyrra_f3.webp",
   "sirius/sirius_fyrra_f4.webp",
   "sirius/sirius_fyrra_f5.webp",
   "sirius/sirius_shaira_f1.webp",
   "sirius/sirius_shaira_f2.webp",
   "sirius/sirius_shaira_f3.webp",
   "sirius/sirius_shaira_f4.webp",
   "sirius/sirius_shaira_f5.webp",
}

portraits_m["Pirate"] = {
   "pirate/pirate1.webp",
   "pirate/pirate2.webp",
   "pirate/pirate3.webp",
   "pirate/pirate4.webp",
   "pirate/pirate7.webp",
   "pirate/pirate8.webp",
   "pirate/pirate9.webp",
   "pirate/pirate10.webp",
   "pirate/pirate11.webp",
   "pirate/pirate12.webp",
   "pirate/pirate13.webp",
   "pirate/pirate_militia1.webp",
   "pirate/pirate_militia2.webp",
}
portraits_f["Pirate"] = {
   "pirate/pirate2.webp",
   "pirate/pirate3.webp",
   "pirate/pirate5.webp",
   "pirate/pirate6.webp",
   "pirate/pirate7.webp",
   "pirate/pirate8.webp",
   "pirate/pirate9.webp",
   "pirate/pirate10.webp",
   "pirate/pirate11.webp",
   "pirate/pirate13.webp",
   "pirate/pirate_militia1.webp",
   "pirate/pirate_militia2.webp",
}

portraits_mil_m["Empire"] = {
   "empire/empire_mil_m1.webp",
   "empire/empire_mil_m2.webp",
   "empire/empire_mil_m3.webp",
   "empire/empire_mil_m4.webp",
   "empire/empire_mil_m5.webp",
   "empire/empire_mil_m6.webp",
   "empire/empire_mil_m7.webp",
   "empire/empire_mil_m8.webp",
}
portraits_mil_f["Empire"] = {
   "empire/empire_mil_f1.webp",
   "empire/empire_mil_f2.webp",
   "empire/empire_mil_f3.webp",
   "empire/empire_mil_f4.webp",
   "empire/empire_mil_f5.webp",
}

portraits_mil_m["Dvaered"] = {
   "dvaered/dv_military_m1.webp",
   "dvaered/dv_military_m2.webp",
   "dvaered/dv_military_m3.webp",
   "dvaered/dv_military_m4.webp",
   "dvaered/dv_military_m5.webp",
   "dvaered/dv_military_m6.webp",
   "dvaered/dv_military_m7.webp",
   "dvaered/dv_military_m8.webp",
}
portraits_mil_f["Dvaered"] = {
   "dvaered/dv_military_f1.webp",
   "dvaered/dv_military_f2.webp",
   "dvaered/dv_military_f3.webp",
   "dvaered/dv_military_f4.webp",
   "dvaered/dv_military_f5.webp",
   "dvaered/dv_military_f6.webp",
   "dvaered/dv_military_f7.webp",
   "dvaered/dv_military_f8.webp",
}

portraits_mil_m["Pirate"] = portraits_m["Pirate"]
portraits_mil_f["Pirate"] = portraits_f["Pirate"]


--[[--
Choose a random male civilian portrait.

@usage misn.setNPC( "Sam", getMale( "Pirate" ), description )
   @tparam[opt="neutral"] string faction Name of faction to get a portrait for, or nil for neutral.
--]]
function portrait.getMale( faction )
   if portraits_m[faction] ~= nil then
      return portraits_m[faction][ rnd.rnd( 1, #portraits_m[faction] ) ]
   else
      return portraits_m_neutral[ rnd.rnd( 1, #portraits_m_neutral ) ]
   end
end


--[[--
Choose a random female civilian portrait.

@usage misn.setNPC( "Sam", getFemale(), description )
   @tparam[opt="neutral"] string faction Name of faction to get a portrait for, or nil for neutral.
--]]
function portrait.getFemale( faction )
   if portraits_f[faction] ~= nil then
      return portraits_f[faction][ rnd.rnd( 1, #portraits_f[faction] ) ]
   else
      return portraits_f_neutral[ rnd.rnd( 1, #portraits_f_neutral ) ]
   end
end


--[[--
Choose a random civilian portrait of any gender.

@usage misn.setNPC( "Sam", get( "Empire" ), description )
   @tparam[opt="neutral"] string faction Name of faction to get a portrait for, or nil for neutral.
--]]
function portrait.get( faction )
   if rnd.rnd() < 0.5 then
      return portrait.getMale( faction )
   else
      return portrait.getFemale( faction )
   end
end


--[[--
Choose a random male military portrait.

@usage misn.setNPC( "Sam", getMaleMil( "Pirate" ), description )
   @tparam[opt="neutral"] string faction Name of faction to get a portrait for, or nil for neutral.
--]]
function portrait.getMaleMil( faction )
   if portraits_mil_m[faction] ~= nil then
      return portraits_mil_m[faction][ rnd.rnd( 1, #portraits_mil_m[faction] ) ]
   else
      return portraits_m_neutral[ rnd.rnd( 1, #portraits_m_neutral ) ]
   end
end


--[[--
Choose a random female military portrait.

@usage misn.setNPC( "Sam", getFemaleMil( "Dvaered" ), description )
   @tparam[opt="neutral"] string faction Name of faction to get a portrait for, or nil for neutral.
--]]
function portrait.getFemaleMil( faction )
   if portraits_mil_f[faction] ~= nil then
      return portraits_mil_f[faction][ rnd.rnd( 1, #portraits_mil_f[faction] ) ]
   else
      return portraits_f_neutral[ rnd.rnd( 1, #portraits_f_neutral ) ]
   end
end


--[[--
Choose a random military portrait of any gender.

@usage misn.setNPC( "Sam", getMil( "Empire" ), description )
   @tparam[opt="neutral"] string faction Name of faction to get a portrait for, or nil for neutral.
--]]
function portrait.getMil( faction )
   if rnd.rnd() < 0.5 then
      return portrait.getMaleMil( faction )
   else
      return portrait.getFemaleMil( faction )
   end
end


--[[--
Gets the full path of a portrait relative to the data directory.

@usage portrait.getFullPath( portrait.get() )
   @tparam string str Name of the portrait image to get.
--]]
function portrait.getFullPath( str )
   return "gfx/portraits/"..str
end


return portrait
