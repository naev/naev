--[[

   Portrait Common

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

--]]
local imageproc = require "imageproc"

local portrait = {}

local portraits_m = {}
local portraits_f = {}
local portraits_mil_m = {}
local portraits_mil_f = {}

local portraits_m_neutral = {
   "neutral/male1.png",
   "neutral/male2.png",
   "neutral/male3.png",
   "neutral/male4.png",
   "neutral/male5.png",
   "neutral/male6.png",
   "neutral/male7.png",
   "neutral/male8.png",
   "neutral/male9.png",
   "neutral/male10.png",
   "neutral/male11.png",
   "neutral/male12.png",
   "neutral/male13.png",
   "neutral/miner1.png",
   "neutral/thief1.png",
   "neutral/thief2.png",
}
local portraits_f_neutral = {
   "neutral/female1.png",
   "neutral/female2.png",
   "neutral/female3.png",
   "neutral/female4.png",
   "neutral/female5.png",
   "neutral/female6.png",
   "neutral/female7.png",
   "neutral/female8.png",
   "neutral/female9.png",
   "neutral/female10.png",
   "neutral/female11.png",
   "neutral/female12.png",
   "neutral/female13.png",
   "neutral/miner2.png",
   "neutral/thief3.png",
   "neutral/thief4.png",
}

portraits_m["Dvaered"] = {
   "dvaered/dv_civilian_m1.png",
   "dvaered/dv_civilian_m2.png",
   "dvaered/dv_civilian_m3.png",
   "dvaered/dv_civilian_m4.png",
   "dvaered/dv_civilian_m5.png",
   "dvaered/dv_civilian_m6.png",
   "dvaered/dv_civilian_m7.png",
   "dvaered/dv_civilian_m8.png",
   "dvaered/dv_civilian_m9.png",
   "dvaered/dv_civilian_m10.png",
   "dvaered/dv_civilian_m11.png",
}
portraits_f["Dvaered"] = {
   "dvaered/dv_civilian_f1.png",
   "dvaered/dv_civilian_f2.png",
   "dvaered/dv_civilian_f3.png",
   "dvaered/dv_civilian_f4.png",
   "dvaered/dv_civilian_f5.png",
   "dvaered/dv_civilian_f6.png",
   "dvaered/dv_civilian_f7.png",
   "dvaered/dv_civilian_f8.png",
   "dvaered/dv_civilian_f9.png",
   "dvaered/dv_civilian_f10.png",
   "dvaered/dv_civilian_f11.png",
}

portraits_m["Sirius"] = {
   "sirius/sirius_fyrra_m1.png",
   "sirius/sirius_fyrra_m2.png",
   "sirius/sirius_fyrra_m3.png",
   "sirius/sirius_fyrra_m4.png",
   "sirius/sirius_fyrra_m5.png",
   "sirius/sirius_shiara_m1.png",
   "sirius/sirius_shiara_m2.png",
   "sirius/sirius_shiara_m3.png",
   "sirius/sirius_shiara_m4.png",
   "sirius/sirius_shiara_m5.png",
}
portraits_f["Sirius"] = {
   "sirius/sirius_fyrra_f1.png",
   "sirius/sirius_fyrra_f2.png",
   "sirius/sirius_fyrra_f3.png",
   "sirius/sirius_fyrra_f4.png",
   "sirius/sirius_fyrra_f5.png",
   "sirius/sirius_shiara_f1.png",
   "sirius/sirius_shiara_f2.png",
   "sirius/sirius_shiara_f3.png",
   "sirius/sirius_shiara_f4.png",
   "sirius/sirius_shiara_f5.png",
}

portraits_m["Pirate"] = {
   "pirate/pirate1.png",
   "pirate/pirate2.png",
   "pirate/pirate3.png",
   "pirate/pirate4.png",
   "pirate/pirate7.png",
   "pirate/pirate8.png",
   "pirate/pirate9.png",
   "pirate/pirate10.png",
   "pirate/pirate11.png",
   "pirate/pirate12.png",
   "pirate/pirate13.png",
   "pirate/pirate_militia1.png",
   "pirate/pirate_militia2.png",
}
portraits_f["Pirate"] = {
   "pirate/pirate2.png",
   "pirate/pirate3.png",
   "pirate/pirate5.png",
   "pirate/pirate6.png",
   "pirate/pirate7.png",
   "pirate/pirate8.png",
   "pirate/pirate9.png",
   "pirate/pirate10.png",
   "pirate/pirate11.png",
   "pirate/pirate13.png",
   "pirate/pirate_militia1.png",
   "pirate/pirate_militia2.png",
}

portraits_mil_m["Empire"] = {
   "empire/empire_mil_m1.png",
   "empire/empire_mil_m2.png",
   "empire/empire_mil_m3.png",
   "empire/empire_mil_m4.png",
   "empire/empire_mil_m5.png",
   "empire/empire_mil_m6.png",
   "empire/empire_mil_m7.png",
   "empire/empire_mil_m8.png",
}
portraits_mil_f["Empire"] = {
   "empire/empire_mil_f1.png",
   "empire/empire_mil_f2.png",
   "empire/empire_mil_f3.png",
   "empire/empire_mil_f4.png",
   "empire/empire_mil_f5.png",
}

portraits_mil_m["Dvaered"] = {
   "dvaered/dv_military_m1.png",
   "dvaered/dv_military_m2.png",
   "dvaered/dv_military_m3.png",
   "dvaered/dv_military_m4.png",
   "dvaered/dv_military_m5.png",
   "dvaered/dv_military_m6.png",
   "dvaered/dv_military_m7.png",
   "dvaered/dv_military_m8.png",
}
portraits_mil_f["Dvaered"] = {
   "dvaered/dv_military_f1.png",
   "dvaered/dv_military_f2.png",
   "dvaered/dv_military_f3.png",
   "dvaered/dv_military_f4.png",
   "dvaered/dv_military_f5.png",
   "dvaered/dv_military_f6.png",
   "dvaered/dv_military_f7.png",
   "dvaered/dv_military_f8.png",
}

portraits_mil_m["Pirate"] = portraits_m["Pirate"]
portraits_mil_f["Pirate"] = portraits_f["Pirate"]


--[[
-- @brief Choose a random male civilian portrait.
--
-- @usage misn.setNPC( "Sam", getMale( "Pirate" ) )
--
--    @luaparam faction Name of faction to get a portrait for, or nil for neutral.
--
-- @luafunc getMale( faction )
--]]
function portrait.getMale( faction )
   if portraits_m[faction] ~= nil then
      return portraits_m[faction][ rnd.rnd( 1, #portraits_m[faction] ) ]
   else
      return portraits_m_neutral[ rnd.rnd( 1, #portraits_m_neutral ) ]
   end
end


--[[
-- @brief Choose a random female civilian portrait.
--
-- @usage misn.setNPC( "Sam", getFemale() )
--
--    @luaparam faction Name of faction to get a portrait for, or nil for neutral.
--
-- @luafunc getFemale( faction )
--]]
function portrait.getFemale( faction )
   if portraits_f[faction] ~= nil then
      return portraits_f[faction][ rnd.rnd( 1, #portraits_f[faction] ) ]
   else
      return portraits_f_neutral[ rnd.rnd( 1, #portraits_f_neutral ) ]
   end
end


--[[
-- @brief Choose a random civilian portrait of any gender.
--
-- @usage misn.setNPC( "Sam", get( "Empire" ) )
--
--    @luaparam faction Name of faction to get a portrait for, or nil for neutral.
--
-- @luafunc get( faction )
--]]
function portrait.get( faction )
   if rnd.rnd() < 0.5 then
      return portrait.getMale( faction )
   else
      return portrait.getFemale( faction )
   end
end


--[[
-- @brief Choose a random male military portrait.
--
-- @usage misn.setNPC( "Sam", getMaleMil( "Pirate" ) )
--
--    @luaparam faction Name of faction to get a portrait for, or nil for neutral.
--
-- @luafunc getMaleMil( faction )
--]]
function portrait.getMaleMil( faction )
   if portraits_mil_m[faction] ~= nil then
      return portraits_mil_m[faction][ rnd.rnd( 1, #portraits_mil_m[faction] ) ]
   else
      return portraits_m_neutral[ rnd.rnd( 1, #portraits_m_neutral ) ]
   end
end


--[[
-- @brief Choose a random female military portrait.
--
-- @usage misn.setNPC( "Sam", getFemaleMil( "Dvaered" ) )
--
--    @luaparam faction Name of faction to get a portrait for, or nil for neutral.
--
-- @luafunc getFemaleMil( faction )
--]]
function portrait.getFemaleMil( faction )
   if portraits_mil_f[faction] ~= nil then
      return portraits_mil_f[faction][ rnd.rnd( 1, #portraits_mil_f[faction] ) ]
   else
      return portraits_f_neutral[ rnd.rnd( 1, #portraits_f_neutral ) ]
   end
end


--[[
-- @brief Choose a random military portrait of any gender.
--
-- @usage misn.setNPC( "Sam", getMil( "Empire" ) )
--
--    @luaparam faction Name of faction to get a portrait for, or nil for neutral.
--
-- @luafunc getMil( faction )
--]]
function portrait.getMil( faction )
   if rnd.rnd() < 0.5 then
      return portrait.getMaleMil( faction )
   else
      return portrait.getFemaleMil( faction )
   end
end


--[[
-- @brief Gets the full path of a portrait relative to the data directory.
--]]
function portrait.getFullPath( str )
   return "gfx/portraits/"..str
end


--[[
-- @brief Gets the hologram image from the portrait.
--]]
function portrait.hologram( str )
   return imageproc.hologram( portrait.getFullPath( str ) )
end


return portrait
