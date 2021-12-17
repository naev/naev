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
}

-- Zach Xiao
zbh.zach = {
   -- TODO proper graphics
   portrait = "zach.webp",
   image = "zach.webp",
   name = _("Zach"),
   colour = nil,
   transition = "hexagon",
   description = _("Zach looks like he is idle at the bar. You wonder what he's thinking about."),
}

function zbh.vn_zach( params )
   return vn.Character.new( zbh.zach.name,
         mt.merge_tables( {
            image=zbh.zach.image,
            color=zbh.zach.colour,
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
}

zbh.fctmod = {
   zbh01 = 2,
   zbh02 = 2,
}

return zbh
