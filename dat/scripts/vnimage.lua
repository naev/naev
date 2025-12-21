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
         local img = lg.newImage( tex.open(file.from_string(svg)) )
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
      width  = 400,
      height = 300,
   }
   -- Increase size based on scale
   PORTRAIT.width = (PORTRAIT.width or 400) / scale
   PORTRAIT.height = (PORTRAIT.height or 300) / scale
   npc.replace = tmergei( npc.replace, {
      { [[width="1000"]], fmt.f([[width="{width}"]], PORTRAIT) },
      { [[height="1415"]], fmt.f([[height="{height}"]], PORTRAIT) },
      { [[viewBox="0 0 1000 1415"]], fmt.f([[viewBox="{viewxs} {viewys} {viewxe} {viewye}" preserveAspectRatio="xMidYMin slice" style="aspect-ratio:{width}/{height};max-height:{height}px;"]], PORTRAIT) },
   } )
   local prt = lg.newCanvas( PORTRAIT.width, PORTRAIT.height )
   render_svgs( prt, path, npc )
   lg.setCanvas()

   -- Due to limitations of the C-side API, we have to pass the texture as a portrait, while the VN supports canvases
   return img.t.tex, prt.t.tex
end

local function gen( path )
   return function ()
      local lpath = "gfx/vn/characters/"..path
      return vni.generator( lpath )
   end
end

local neutral_m = {
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
   -- New images
   {"neutral/male1n"},
   {"neutral/male1n_v2"},
   {"neutral/male1n_v3"},
   {"neutral/male1n_v4"},
   {"neutral/male2n"},
   {"neutral/male3n"},
   {"neutral/male3n_v2"},
   -- Image generators
   gen( "neutral/male01" ),
}
local neutral_f = {
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
   -- New images
   {"neutral/female1n"},
   {"neutral/female1n_v2"},
   {"neutral/female1n_v3"},
   {"neutral/female1n_v4"},
   {"neutral/female1n_v5"},
   {"neutral/female2n"},
   {"neutral/female2n_nogog"},
   {"neutral/female3n"},
   {"neutral/female4n"},
   {"neutral/female5n"},
   {"neutral/female6n"},
   {"neutral/female7n"},
   {"neutral/female7n_v2"},
   {"neutral/female7n_v3"},
   -- Image generators
   gen( "neutral/female01" ),
   gen( "neutral/female05" ),
   gen( "neutral/female07" ),
}
vni.genericMale = get_list( neutral_m )
vni.genericFemale = get_list( neutral_f )
vni.generic = get_list( neutral_m, neutral_f )
local neutral_new = {
   {"neutral/male1n"},
   {"neutral/male2n"},
   {"neutral/male3n"},
   {"neutral/male3n_v2"},
   gen( "neutral/male01" ),
   {"neutral/female1n"},
   {"neutral/female1n_v2"},
   {"neutral/female1n_v3"},
   {"neutral/female1n_v4"},
   {"neutral/female1n_v5"},
   {"neutral/female2n"},
   {"neutral/female2n_nogog"},
   {"neutral/female3n"},
   {"neutral/female4n"},
   {"neutral/female5n"},
   {"neutral/female6n"},
   {"neutral/female7n"},
   {"neutral/female7n_v2"},
   {"neutral/female7n_v3"},
   gen( "neutral/female01" ),
   gen( "neutral/female07" ),
}
-- Temporary function until we only have new images
vni.genericNew = get_list( neutral_new )

vni.trader = vni.generic
vni.traderMale = vni.genericMale
vni.traderFemale = vni.genericFemale

local pirate_m = {
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
local pirate_f = {
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
vni.pirateMale = get_list( pirate_m )
vni.pirateFemale = get_list( pirate_f )
vni.pirate = get_list( pirate_m, pirate_f )

vni.empire = vni.generic
vni.empireMale = vni.genericMale
vni.empireFemale = vni.genericFemale
local empire_mil_m = {
   "empire/empire_mil_m1",
   "empire/empire_mil_m2",
   "empire/empire_mil_m3",
   "empire/empire_mil_m4",
   "empire/empire_mil_m5",
   "empire/empire_mil_m6",
   "empire/empire_mil_m7",
   "empire/empire_mil_m8",
}
local empire_mil_f = {
   "empire/empire_mil_f1",
   "empire/empire_mil_f2",
   "empire/empire_mil_f3",
   "empire/empire_mil_f4",
   "empire/empire_mil_f5",
}
vni.empireMilitaryMale = get_list( empire_mil_m )
vni.empireMilitaryFemale = get_list( empire_mil_f )
vni.empireMilitary = get_list( empire_mil_m, empire_mil_f )

local dvaered_m = {
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
local dvaered_f = {
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
vni.dvaeredMale = get_list( dvaered_m )
vni.dvaeredFemale = get_list( dvaered_f )
vni.dvaered = get_list( dvaered_m, dvaered_f )

local dvaered_mil_m = {
   "dvaered/dv_military_m1",
   "dvaered/dv_military_m2",
   "dvaered/dv_military_m3",
   "dvaered/dv_military_m4",
   "dvaered/dv_military_m5",
   "dvaered/dv_military_m6",
   "dvaered/dv_military_m7",
   "dvaered/dv_military_m8",
}
local dvaered_mil_f = {
   "dvaered/dv_military_f1",
   "dvaered/dv_military_f2",
   "dvaered/dv_military_f3",
   "dvaered/dv_military_f4",
   "dvaered/dv_military_f5",
   "dvaered/dv_military_f6",
   "dvaered/dv_military_f7",
   "dvaered/dv_military_f8",
}
vni.dvaeredMilitaryMale = get_list( dvaered_mil_m )
vni.dvaeredMilitaryFemale = get_list( dvaered_mil_f )
vni.dvaeredMilitary = get_list( dvaered_mil_m, dvaered_mil_f )

local soromid = {
   {"soromid/soromid_heavy_civilian_1", "heavy"},
   {"soromid/soromid_heavy_civilian_2", "heavy"},
   {"soromid/soromid_heavy_civilian_3", "heavy"},
   {"soromid/soromid_aquatic_01", "aquatic"},
   {"soromid/soromid_aquatic_01_v2", "aquatic"},
   {"soromid/soromid_aquatic_01_v3", "aquatic"},
   {"soromid/soromid_dark_01", "dark"},
   {"soromid/soromid_dark_01_v2", "dark"},
   {"soromid/soromid_dark_01_v3", "dark"},
   -- Image Generators
   {gen("soromid/soromid_dark_01"), "dark"},
}
local soromid_mil = {
   {"soromid/soromid_heavy_military_3", "heavy"},
}
local function get_soromid( list, species )
   if species == nil then
      return _get_list( list )
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
   "sirius/sirius_fyrra_m1",
   "sirius/sirius_fyrra_m2",
   "sirius/sirius_fyrra_m3",
   "sirius/sirius_fyrra_m4",
   "sirius/sirius_fyrra_m5",
   {'sirius/sirius_fyrra_m1n'},
   {'sirius/sirius_fyrra_m1_v2'},
   {'sirius/sirius_fyrra_m1_v3'},
   {'sirius/sirius_fyrra_m1_v4'},
}
local sirius_shaira_m = {
   "sirius/sirius_shaira_m1",
   "sirius/sirius_shaira_m2",
   "sirius/sirius_shaira_m3",
   "sirius/sirius_shaira_m4",
   "sirius/sirius_shaira_m5",
}
local sirius_serra_m = {
   "sirius/sirius_serra_m1",
   "sirius/sirius_serra_m2",
   "sirius/sirius_serra_m3",
   "sirius/sirius_serra_m4",
}
local sirius_fyrra_f = {
   "sirius/sirius_fyrra_f1",
   "sirius/sirius_fyrra_f2",
   "sirius/sirius_fyrra_f3",
   "sirius/sirius_fyrra_f4",
   "sirius/sirius_fyrra_f5",
}
local sirius_shaira_f = {
   "sirius/sirius_shaira_f1",
   "sirius/sirius_shaira_f2",
   "sirius/sirius_shaira_f3",
   "sirius/sirius_shaira_f4",
   "sirius/sirius_shaira_f5",
}
local sirius_serra_f = {
   "sirius/sirius_serra_f1",
   "sirius/sirius_serra_f2",
   "sirius/sirius_serra_f3",
   "sirius/sirius_serra_f4",
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
   lg.printf( id, fl, 0, 200, 1000, "centre" )
   lg.printf( p_("vn_extras", "SOUND ONLY"), fs, 0, 550, 1000, "centre" )
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
   lg.printf( id, fl, 0, 200, 1000, "centre" )
   lg.printf( p_("vn_extras", "TEXT ONLY"), fs, 0, 550, 1000, "centre" )
   lg.setCanvas( oc )

   return vn.Character.new(
         fmt.f(_("CHANNEL {id}"),{id=id}),
         tmerge( {image=c, flip=false}, params ) )
end

return vni
