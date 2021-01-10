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
   "neutral/male1",
   "neutral/male2",
   "neutral/male3",
   "neutral/male4",
   "neutral/male5",
   "neutral/male6",
   "neutral/male7",
   "neutral/male8",
   "neutral/male9",
   "neutral/male10",
   "neutral/male11",
   "neutral/male12",
   "neutral/male13",
   "neutral/miner1",
   "neutral/thief1",
   "neutral/thief2",
}
local portraits_f_neutral = {
   "neutral/female1",
   "neutral/female2",
   "neutral/female3",
   "neutral/female4",
   "neutral/female5",
   "neutral/female6",
   "neutral/female7",
   "neutral/female8",
   "neutral/female9",
   "neutral/female10",
   "neutral/female11",
   "neutral/female12",
   "neutral/female13",
   "neutral/miner2",
   "neutral/thief3",
   "neutral/thief4",
}

portraits_m["Dvaered"] = {
   "dvaered/dv_civilian_m1",
   "dvaered/dv_civilian_m2",
   "dvaered/dv_civilian_m3",
   "dvaered/dv_civilian_m4",
   "dvaered/dv_civilian_m5",
   "dvaered/dv_civilian_m6",
   "dvaered/dv_civilian_m7",
   "dvaered/dv_civilian_m8",
   "dvaered/dv_civilian_m9",
   "dvaered/dv_civilian_m10",
   "dvaered/dv_civilian_m11",
}
portraits_f["Dvaered"] = {
   "dvaered/dv_civilian_f1",
   "dvaered/dv_civilian_f2",
   "dvaered/dv_civilian_f3",
   "dvaered/dv_civilian_f4",
   "dvaered/dv_civilian_f5",
   "dvaered/dv_civilian_f6",
   "dvaered/dv_civilian_f7",
   "dvaered/dv_civilian_f8",
   "dvaered/dv_civilian_f9",
   "dvaered/dv_civilian_f10",
   "dvaered/dv_civilian_f11",
}

portraits_m["Sirius"] = {
   "sirius/sirius_fyrra_m1",
   "sirius/sirius_fyrra_m2",
   "sirius/sirius_fyrra_m3",
   "sirius/sirius_fyrra_m4",
   "sirius/sirius_fyrra_m5",
   "sirius/sirius_shiara_m1",
   "sirius/sirius_shiara_m2",
   "sirius/sirius_shiara_m3",
   "sirius/sirius_shiara_m4",
   "sirius/sirius_shiara_m5",
}
portraits_f["Sirius"] = {
   "sirius/sirius_fyrra_f1",
   "sirius/sirius_fyrra_f2",
   "sirius/sirius_fyrra_f3",
   "sirius/sirius_fyrra_f4",
   "sirius/sirius_fyrra_f5",
   "sirius/sirius_shiara_f1",
   "sirius/sirius_shiara_f2",
   "sirius/sirius_shiara_f3",
   "sirius/sirius_shiara_f4",
   "sirius/sirius_shiara_f5",
}

portraits_m["Pirate"] = {
   "pirate/pirate1",
   "pirate/pirate2",
   "pirate/pirate3",
   "pirate/pirate4",
   "pirate/pirate7",
   "pirate/pirate8",
   "pirate/pirate9",
   "pirate/pirate10",
   "pirate/pirate11",
   "pirate/pirate12",
   "pirate/pirate13",
   "pirate/pirate_militia1",
   "pirate/pirate_militia2",
}
portraits_f["Pirate"] = {
   "pirate/pirate2",
   "pirate/pirate3",
   "pirate/pirate5",
   "pirate/pirate6",
   "pirate/pirate7",
   "pirate/pirate8",
   "pirate/pirate9",
   "pirate/pirate10",
   "pirate/pirate11",
   "pirate/pirate13",
   "pirate/pirate_militia1",
   "pirate/pirate_militia2",
}

portraits_mil_m["Empire"] = {
   "empire/empire_mil_m1",
   "empire/empire_mil_m2",
   "empire/empire_mil_m3",
   "empire/empire_mil_m4",
   "empire/empire_mil_m5",
   "empire/empire_mil_m6",
   "empire/empire_mil_m7",
   "empire/empire_mil_m8",
}
portraits_mil_f["Empire"] = {
   "empire/empire_mil_f1",
   "empire/empire_mil_f2",
   "empire/empire_mil_f3",
   "empire/empire_mil_f4",
   "empire/empire_mil_f5",
}

portraits_mil_m["Dvaered"] = {
   "dvaered/dv_military_m1",
   "dvaered/dv_military_m2",
   "dvaered/dv_military_m3",
   "dvaered/dv_military_m4",
   "dvaered/dv_military_m5",
   "dvaered/dv_military_m6",
   "dvaered/dv_military_m7",
   "dvaered/dv_military_m8",
}
portraits_mil_f["Dvaered"] = {
   "dvaered/dv_military_f1",
   "dvaered/dv_military_f2",
   "dvaered/dv_military_f3",
   "dvaered/dv_military_f4",
   "dvaered/dv_military_f5",
   "dvaered/dv_military_f6",
   "dvaered/dv_military_f7",
   "dvaered/dv_military_f8",
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
   return "gfx/portraits/"..str..".png"
end


--[[
-- @brief Gets the hologram image from the portrait.
--]]
function portrait.hologram( str )
   return imageproc.hologram( portrait.getFullPath( str ) )
end


return portrait
