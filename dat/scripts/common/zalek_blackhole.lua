--[[

   Za'lek Black hole Common Functions

--]]
local vn = require "vn"
local mt = require 'merge_tables'
local audio = require "love.audio"

local zbh = {}

zbh.sfx = {
   spacewhale1 = audio.newSource( "snd/sounds/spacewhale1.ogg" ),
   spacewhale2 = audio.newSource( "snd/sounds/spacewhale2.ogg" ),
   bite = audio.newSource( "snd/sounds/crash1.ogg" ),
}

-- Zach Xiao
zbh.zach = {
   portrait = "zach.webp",
   image = "zach.webp",
   name = _("Zach"),
   colour = nil,
   transition = "hexagon",
   description = _("Zach looks like he is idle at the bar. You wonder what he's thinking about."),
}
zbh.icarus = {
   name = _("Feral Bioship"),
   portrait = nil,
   -- TODO change image to proper one when Nohinohi graphics are changed
   image = 'gfx/ship/reaver/reaver_comm.webp',
   colour = nil,
}

function zbh.vn_zach( params )
   return vn.Character.new( zbh.zach.name,
         mt.merge_tables( {
            image=zbh.zach.image,
            color=zbh.zach.colour,
         }, params) )
end

function zbh.vn_icarus( params )
   return vn.Character.new( zbh.icarus.name,
         mt.merge_tables( {
            image=zbh.icarus.image,
            color=zbh.icarus.colour,
         }, params) )
end

function zbh.log( text )
   shiplog.create( "zlk_blackhole", _("Black Hole"), _("Za'lek") )
   shiplog.append( "zlk_blackhole", text )
end

zbh.unidiff_list = {
   "sigma13_fixed1",
   "sigma13_fixed2",
}

--[[
   Evil Principal Investigator (PI) Faction
--]]
function zbh.evilpi ()
   local f = faction.exists("evilpi")
   if f then
      return f
   end
   return faction.dynAdd( "Za'lek", "evilpi", _("Evil PI"),
         {clear_enemies=true, clear_allies=true} )
end

function zbh.plt_icarus( pos )
   local fct = faction.exists("feralbioship")
   if not fct then
      fct = faction.dynAdd( nil, "feralbioship", _("Feral Bioship"), {ai="dummy"} )
   end
   return pilot.add( "Nohinohi", fct, pos, _("Icarus") )
end

function zbh.unidiff( diffname )
   for _k,d in ipairs(zbh.unidiff_list) do
      if diff.isApplied(d) then
         diff.remove(d)
      end
   end
   diff.apply( diffname )
end

zbh.rewards = {
   zbh01 = 200e3,
   zbh02 = 300e3,
   zbh03 = 400e3,
   zbh04 = 300e3,
   zbh05 = 600e3,
}

zbh.fctmod = {
   zbh01 = 2,
   zbh02 = 2,
   zbh03 = 2,
   zbh04 = 2,
   zbh05 = 3,
}

return zbh
