--[[--
Functions for handling vnis in Naev.

@module vnimage
--]]
local portrait = require "portrait"

local vn = require "vn"
local lg = require "love.graphics"
local fmt = require "format"

local vni = {}

local neutral_m = {
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
   -- New images
   {"neutral/male1n.webp"},
   {"neutral/male2n.webp"},
   {"neutral/male3n.webp"},
   {"neutral/male3n_v2.webp"},
}
local neutral_f = {
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
   -- New images
   {"neutral/female1n.webp"},
   {"neutral/female2n.webp"},
   {"neutral/female2n_nogog.webp"},
   {"neutral/female3n.webp"},
   {"neutral/female4n.webp"},
}
function vni.genericMale()
   local p = neutral_m[ rnd.rnd(1,#neutral_m) ]
   if type(p)=="table" then
      return p[1], p[1]
   end
   return portrait.getFullPath(p), p
end
function vni.genericFemale()
   local p = neutral_f[ rnd.rnd(1,#neutral_f) ]
   if type(p)=="table" then
      return p[1], p[1]
   end
   return portrait.getFullPath(p), p
end
function vni.generic()
   if rnd.rnd() < 0.5 then
      return vni.genericFemale()
   end
   return vni.genericMale()
end

local pirate_m = {
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
local pirate_f = {
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
function vni.pirateMale()
   local p = pirate_m[ rnd.rnd(1,#pirate_m) ]
   return portrait.getFullPath(p), p
end
function vni.pirateFemale()
   local p = pirate_f[ rnd.rnd(1,#pirate_f) ]
   return portrait.getFullPath(p), p
end
function vni.pirate()
   if rnd.rnd() < 0.5 then
      return vni.pirateFemale()
   end
   return vni.pirateMale()
end

local sirius_fyrra_m = {
   "sirius/sirius_fyrra_m1.webp",
   "sirius/sirius_fyrra_m2.webp",
   "sirius/sirius_fyrra_m3.webp",
   "sirius/sirius_fyrra_m4.webp",
   "sirius/sirius_fyrra_m5.webp",
}
local sirius_shaira_m = {
   "sirius/sirius_shaira_m1.webp",
   "sirius/sirius_shaira_m2.webp",
   "sirius/sirius_shaira_m3.webp",
   "sirius/sirius_shaira_m4.webp",
   "sirius/sirius_shaira_m5.webp",
}
local sirius_fyrra_f = {
   "sirius/sirius_fyrra_f1.webp",
   "sirius/sirius_fyrra_f2.webp",
   "sirius/sirius_fyrra_f3.webp",
   "sirius/sirius_fyrra_f4.webp",
   "sirius/sirius_fyrra_f5.webp",
}
local sirius_shaira_f = {
   "sirius/sirius_shaira_f1.webp",
   "sirius/sirius_shaira_f2.webp",
   "sirius/sirius_shaira_f3.webp",
   "sirius/sirius_shaira_f4.webp",
   "sirius/sirius_shaira_f5.webp",
}
vni.sirius = {}
function vni.sirius.shairaMale()
   local p = sirius_shaira_m[ rnd.rnd(1,#sirius_shaira_m) ]
   return portrait.getFullPath(p), p
end
function vni.sirius.fyrraMale()
   local p = sirius_fyrra_m[ rnd.rnd(1,#sirius_fyrra_m) ]
   return portrait.getFullPath(p), p
end
function vni.sirius.serraMale()
   -- TODO
   --local p = sirius_serra_m[ rnd.rnd(1,#sirius_serra_m) ]
   --return portrait.getFullPath(p), p
   return vni.sirius.fyrraMale()
end
function vni.sirius.shairaFemale()
   local p = sirius_shaira_f[ rnd.rnd(1,#sirius_shaira_f) ]
   return portrait.getFullPath(p), p
end
function vni.sirius.fyrraFemale()
   local p = sirius_fyrra_f[ rnd.rnd(1,#sirius_fyrra_f) ]
   return portrait.getFullPath(p), p
end
function vni.sirius.serraFemale()
   -- TODO
   --local p = sirius_serra_f[ rnd.rnd(1,#sirius_serra_f) ]
   --return portrait.getFullPath(p), p
   return vni.sirius.fyrraFemale()
end
function vni.sirius.shaira()
   if rnd.rnd() < 0.5 then
      return vni.sirius.shairaFemale()
   end
   return vni.sirius.shairaMale()
end
function vni.sirius.fyrra()
   if rnd.rnd() < 0.5 then
      return vni.sirius.fyrraFemale()
   end
   return vni.sirius.fyrraMale()
end
function vni.sirius.serra()
   if rnd.rnd() < 0.5 then
      return vni.sirius.serraFemale()
   end
   return vni.sirius.serraMale()
end

--[[--
Creates a "SOUND ONLY" character for the VN.
   @tparam string id ID of the voice to add.
   @tparam table params Optional parameters to pass or overwrite.
   @treturn vn.Character A new vn character you can add with `vn.newCharacter`.
--]]
function vni.soundonly( id, params )
   params = params or {}
   local c = lg.newCanvas( 1000, 1415 )
   local oc = lg.getCanvas()
   local fl = lg.newFont( "fonts/D2CodingBold.ttf", 300 )
   local fs = lg.newFont( 64 )
   local col = params.color or { 1, 0, 0 }
   lg.setCanvas( c )
   lg.clear{ 0, 0, 0, 0.8 }
   lg.setColor( col )
   lg.printf( id, fl, 0, 200, 1000, "center" )
   lg.printf( p_("vn_extras", "SOUND ONLY"), fs, 0, 550, 1000, "center" )
   lg.setCanvas( oc )

   return vn.Character.new(
         fmt.f(_("VOICE {id}"),{id=id}),
         tmerge( {image=c, flip=false}, params ) )
end

return vni
