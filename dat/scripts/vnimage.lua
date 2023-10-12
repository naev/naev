--[[--
Functions for handling vnis in Naev.

@module vnimage
--]]
local portrait = require "portrait"

local vn = require "vn"
local lg = require "love.graphics"
local fmt = require "format"

local vni = {}

local function _get_list( lst )
   local p = lst[ rnd.rnd(1,#lst) ]
   if type(p)=="table" then
      return p[1], p[1]
   end
   return portrait.getFullPath(p), p
end

local function get_list( ... )
   local arg = {...}
   local n = #arg
   if n==1 then
      return function ()
         return _get_list( arg[1] )
      end
   end
   return function ()
      local r = rnd.rnd()
      for k,v in ipairs(arg) do
         if r <= k/n then
            return _get_list( v )
         end
      end
   end
end

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
   {"neutral/female5n.webp"},
   {"neutral/female6n.webp"},
}
vni.genericMale = get_list( neutral_m )
vni.genericFemale = get_list( neutral_f )
vni.generic = get_list( neutral_m, neutral_f )

vni.trader = vni.generic
vni.traderMale = vni.genericMale
vni.traderFemale = vni.genericFemale

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
vni.pirateMale = get_list( pirate_m )
vni.pirateFemale = get_list( pirate_f )
vni.pirate = get_list( pirate_m, pirate_f )

vni.empire = vni.generic
vni.empireMale = vni.genericMale
vni.empireFemale = vni.genericFemale
local empire_mil_m = {
   "empire/empire_mil_m1.webp",
   "empire/empire_mil_m2.webp",
   "empire/empire_mil_m3.webp",
   "empire/empire_mil_m4.webp",
   "empire/empire_mil_m5.webp",
   "empire/empire_mil_m6.webp",
   "empire/empire_mil_m7.webp",
   "empire/empire_mil_m8.webp",
}
local empire_mil_f = {
   "empire/empire_mil_f1.webp",
   "empire/empire_mil_f2.webp",
   "empire/empire_mil_f3.webp",
   "empire/empire_mil_f4.webp",
   "empire/empire_mil_f5.webp",
}
vni.empireMilitaryMale = get_list( empire_mil_m )
vni.empireMilitaryFemale = get_list( empire_mil_f )
vni.empireMilitary = get_list( empire_mil_m, empire_mil_f )

local dvaered_m = {
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
local dvaered_f = {
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
vni.dvaeredMale = get_list( dvaered_m )
vni.dvaeredFemale = get_list( dvaered_f )
vni.dvaered = get_list( dvaered_m, dvaered_f )

local dvaered_mil_m = {
   "dvaered/dv_military_m1.webp",
   "dvaered/dv_military_m2.webp",
   "dvaered/dv_military_m3.webp",
   "dvaered/dv_military_m4.webp",
   "dvaered/dv_military_m5.webp",
   "dvaered/dv_military_m6.webp",
   "dvaered/dv_military_m7.webp",
   "dvaered/dv_military_m8.webp",
}
local dvaered_mil_f = {
   "dvaered/dv_military_f1.webp",
   "dvaered/dv_military_f2.webp",
   "dvaered/dv_military_f3.webp",
   "dvaered/dv_military_f4.webp",
   "dvaered/dv_military_f5.webp",
   "dvaered/dv_military_f6.webp",
   "dvaered/dv_military_f7.webp",
   "dvaered/dv_military_f8.webp",
}
vni.dvaeredMilitaryMale = get_list( dvaered_mil_m )
vni.dvaeredMilitaryFemale = get_list( dvaered_mil_f )
vni.dvaeredMilitary = get_list( dvaered_mil_m, dvaered_mil_f )

vni.soromid = vni.generic
vni.soromidMale = vni.genericMale
vni.soromidFemale = vni.genericFemale

vni.zalek = vni.generic
vni.zalekMale = vni.genericMale
vni.zalekFemale = vni.genericFemale

local sirius_fyrra_m = {
   "sirius/sirius_fyrra_m1.webp",
   "sirius/sirius_fyrra_m2.webp",
   "sirius/sirius_fyrra_m3.webp",
   "sirius/sirius_fyrra_m4.webp",
   "sirius/sirius_fyrra_m5.webp",
   {'sirius/sirius_fyrra_m1n.webp'},
   {'sirius/sirius_fyrra_m1_v2.webp'},
   {'sirius/sirius_fyrra_m1_v3.webp'},
   {'sirius/sirius_fyrra_m1_v4.webp'},
}
local sirius_shaira_m = {
   "sirius/sirius_shaira_m1.webp",
   "sirius/sirius_shaira_m2.webp",
   "sirius/sirius_shaira_m3.webp",
   "sirius/sirius_shaira_m4.webp",
   "sirius/sirius_shaira_m5.webp",
}
local sirius_serra_m = {
   "sirius/sirius_serra_m1.webp",
   "sirius/sirius_serra_m2.webp",
   "sirius/sirius_serra_m3.webp",
   "sirius/sirius_serra_m4.webp",
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
local sirius_serra_f = {
   "sirius/sirius_serra_f1.webp",
   "sirius/sirius_serra_f2.webp",
   "sirius/sirius_serra_f3.webp",
   "sirius/sirius_serra_f4.webp",
}
vni.sirius = {}
vni.sirius.shairaMale = get_list( sirius_shaira_m )
vni.sirius.fyrraMale = get_list( sirius_fyrra_m )
vni.sirius.serraMale = get_list( sirius_serra_m )

vni.sirius.shairaFemale = get_list( sirius_shaira_f )
vni.sirius.fyrraFemale = get_list( sirius_fyrra_f )
vni.sirius.serraFemale = get_list( sirius_serra_f )

vni.sirius.shaira = get_list( sirius_shaira_m, sirius_shaira_f )
vni.sirius.fyrra = get_list( sirius_fyrra_m, sirius_fyrra_f )
vni.sirius.serra = get_list( sirius_serra_m, sirius_serra_f )

vni.sirius.anyMale = get_list( sirius_shaira_m, sirius_fyrra_m, sirius_serra_m )
vni.sirius.anyFemale = get_list( sirius_shaira_f, sirius_fyrra_f, sirius_serra_f )
vni.sirius.any = get_list( sirius_shaira_m, sirius_fyrra_m, sirius_serra_m, sirius_shaira_f, sirius_fyrra_f, sirius_serra_f )

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
