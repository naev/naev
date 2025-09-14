--[[--
Functions for handling vnis in Naev.

@module vnimage
--]]
local portrait = require "portrait"

local vn = require "vn"
local lg = require "love.graphics"
local lf = require "love.filesystem"
local fmt = require "format"
local pir = require "common.pirate"

local vni = {}

local function _get_list( lst )
   local p = lst[ rnd.rnd(1,#lst) ]
   local t = type(p)
   if t=="table" then
      return p[1], p[1]
   elseif t=="function" then
      return p()
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

-- Renders a clump of SVGs into a single canvas
local function render_svgs( c, path, npc )
   lg.setColour( 1, 1, 1, 1 )
   lg.setCanvas( c )
   lg.clear( 0, 0, 0, 0 )
   for k,v in ipairs(npc) do
      if v then
         local filename = path.."/"..v
         local svg = lf.read( filename )
         if not svg then
            error(fmt.f("svg '{filename}' not found", {filename=filename}))
         end
         for i,r in ipairs(npc.replace) do
            svg = string.gsub( svg, r[1], r[2] )
         end
         local img = lg.newImage( tex.new(file.from_string(svg)) )
         img:draw( 0, 0 )
      end
   end
end

-- Generates an NPC from a generating folder.
-- In particular, it looks up metadata.lua in the folder, and uses that to dynamically generate NPCs.
-- The metadata.lua file should return a function that returns a table with:
--  1. A replace field that specifies how to do SVG file substitutions
--  2. An ordered list of file names (relative to the directory) of files to render.
function vni.generator( path )
   local meta = require(path:gsub("/",".")..".metadata")
   local npc = meta()

   -- The base image is easy, they are all 1000 by 1415 graphics
   -- TODO scale if necessary
   local img = lg.newCanvas( 1000, 1415 )
   render_svgs( img, path, npc )

   -- The portraits are a bit trickier as they are cropped from the original
   -- TODO allow defining the crops in metadata.lua
   local _w, _h, scale = gfx.dim()
   local PORTRAIT = npc.portraitview or {
      -- Viewport is reference to the image coordinates
      viewxs = 100,
      viewys = 0,
      viewxe = 100+800,
      viewye = 0+600,
      -- Width and height are final render
      width  = 400 / scale,
      height = 300 / scale,
   }
   npc.replace = tmergei( npc.replace, {
      { [[width="1000"]], fmt.f([[width="{width}"]], PORTRAIT) },
      { [[height="1415"]], fmt.f([[height="{height}"]], PORTRAIT) },
      { [[viewBox="0 0 1000 1415"]], fmt.f([[viewBox="{viewxs} {viewys} {viewxe} {viewye}" preserveAspectRatio="xMidYMin slice" style="aspect-ratio:{width}/{height};max-height:{height}px;"]], PORTRAIT) },
   } )
   local prt = lg.newCanvas( PORTRAIT.width, PORTRAIT.height )
   render_svgs( prt, path, npc )
   lg.setCanvas()

   -- Due to limitations of the C-side API, we have to pass the texture as a portrait, while the VN supports canvases
   return img, prt.t.tex
end

local function gen( path )
   return function ()
      local lpath = "gfx/vn/characters/"..path
      return vni.generator( lpath )
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
   {"neutral/male1n_v2.webp"},
   {"neutral/male1n_v3.webp"},
   {"neutral/male1n_v4.webp"},
   {"neutral/male2n.webp"},
   {"neutral/male3n.webp"},
   {"neutral/male3n_v2.webp"},
   -- Image generators
   gen( "neutral/male01" ),
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
   {"neutral/female1n_v2.webp"},
   {"neutral/female1n_v3.webp"},
   {"neutral/female1n_v4.webp"},
   {"neutral/female1n_v5.webp"},
   {"neutral/female2n.webp"},
   {"neutral/female2n_nogog.webp"},
   {"neutral/female3n.webp"},
   {"neutral/female4n.webp"},
   {"neutral/female5n.webp"},
   {"neutral/female6n.webp"},
   {"neutral/female7n.webp"},
   {"neutral/female7n_v2.webp"},
   {"neutral/female7n_v3.webp"},
   -- Image generators
   gen( "neutral/female07" ),
}
vni.genericMale = get_list( neutral_m )
vni.genericFemale = get_list( neutral_f )
vni.generic = get_list( neutral_m, neutral_f )
local neutral_new = {
   {"neutral/male1n.webp"},
   {"neutral/male2n.webp"},
   {"neutral/male3n.webp"},
   {"neutral/male3n_v2.webp"},
   gen( "neutral/male01" ),
   {"neutral/female1n.webp"},
   {"neutral/female1n_v2.webp"},
   {"neutral/female1n_v3.webp"},
   {"neutral/female1n_v4.webp"},
   {"neutral/female1n_v5.webp"},
   {"neutral/female2n.webp"},
   {"neutral/female2n_nogog.webp"},
   {"neutral/female3n.webp"},
   {"neutral/female4n.webp"},
   {"neutral/female5n.webp"},
   {"neutral/female6n.webp"},
   {"neutral/female7n.webp"},
   {"neutral/female7n_v2.webp"},
   {"neutral/female7n_v3.webp"},
   gen( "neutral/female07" ),
}
-- Temporary function until we only have new images
vni.genericNew = get_list( neutral_new )

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

local soromid = {
   {"soromid/soromid_heavy_civilian_1.webp", "heavy"},
   {"soromid/soromid_heavy_civilian_2.webp", "heavy"},
   {"soromid/soromid_heavy_civilian_3.webp", "heavy"},
   {"soromid/soromid_aquatic_01.webp", "aquatic"},
   {"soromid/soromid_aquatic_01_v2.webp", "aquatic"},
   {"soromid/soromid_aquatic_01_v3.webp", "aquatic"},
   {"soromid/soromid_dark_01.webp", "dark"},
   {"soromid/soromid_dark_01_v2.webp", "dark"},
   {"soromid/soromid_dark_01_v3.webp", "dark"},
}
local soromid_mil = {
   {"soromid/soromid_heavy_military_3.webp", "heavy"},
}
local function get_soromid( list, species )
   if species == nil then
      local p = list[ rnd.rnd(1,#list) ]
      return p[1], p[1], p[2]
   end
   local valid = {}
   for k,v in ipairs(list) do
      if v[2]==species then
         valid[#valid+1] = v
      end
   end
   if #valid==0 then
      warn(fmt.f("No graphics found for Soromid species '{spc}'",
         {spc=species}))
      return vni.generic()
   end
   return _get_list( valid )
end

vni.soromid = function (species)
   return get_soromid( soromid, species )
end
vni.soromidMilitary = function (species)
   return get_soromid( soromid_mil, species )
end

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
Gets a random portrait fitting for a specific faction.
   @tparam fct Faction fct Faction to get images for.
   @treturn string Path of the VN image.
   @treturn string Path of the portrait image.
--]]
function vni.faction( fct )
   if fct == faction.get("Empire") then
      return vni.empire()
   elseif fct == faction.get("Dvaered") then
      return vni.dvaered()
   elseif fct == faction.get("Za'lek") then
      return vni.zalek()
   elseif fct == faction.get("Soromid") then
      return vni.soromid()
   elseif fct == faction.get("Sirius") then
      return vni.sirius.any()
   elseif pir.factionIsPirate( fct ) then
      return vni.pirate()
   elseif fct == faction.get("Traders Society") then
      return vni.trader()
   else
      return vni.generic()
   end
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
   local col = params.colour or { 1, 0, 0 }
   lg.setCanvas( c )
   lg.clear{ 0, 0, 0, 0.8 }
   lg.setColour( col )
   lg.printf( id, fl, 0, 200, 1000, "center" )
   lg.printf( p_("vn_extras", "SOUND ONLY"), fs, 0, 550, 1000, "center" )
   lg.setCanvas( oc )

   return vn.Character.new(
         fmt.f(_("VOICE {id}"),{id=id}),
         tmerge( {image=c, flip=false}, params ) )
end

--[[--
Creates a "TEXT ONLY" character for the VN.
   @tparam string id ID of the voice to add.
   @tparam table params Optional parameters to pass or overwrite.
   @treturn vn.Character A new vn character you can add with `vn.newCharacter`.
--]]
function vni.textonly( id, params )
   params = params or {}
   local c = lg.newCanvas( 1000, 1415 )
   local oc = lg.getCanvas()
   local fl = lg.newFont( "fonts/D2CodingBold.ttf", 300 )
   local fs = lg.newFont( 64 )
   local col = params.colour or { 1, 0, 0 }
   lg.setCanvas( c )
   lg.clear{ 0, 0, 0, 0.8 }
   lg.setColour( col )
   lg.printf( id, fl, 0, 200, 1000, "center" )
   lg.printf( p_("vn_extras", "TEXT ONLY"), fs, 0, 550, 1000, "center" )
   lg.setCanvas( oc )

   return vn.Character.new(
         fmt.f(_("CHANNEL {id}"),{id=id}),
         tmerge( {image=c, flip=false}, params ) )
end

return vni
