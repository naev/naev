--[[

   Za'lek Black hole Common Functions

--]]
local vn = require "vn"
local mt = require 'merge_tables'

local zbh = {}

-- Zach Xiao
zbh.zach = {
   -- TODO proper graphics
   portrait = "zalek2.png",
   image = "zalek2.png",
   name = _("Zach"),
   color = nil,
   transition = nil, -- Use default
   description = _("You see Noona who looks like she might have a job for you."),
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

zbh.rewards = {
   zbh01 = 200e3,
}

return zbh
