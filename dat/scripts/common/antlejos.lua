--[[

   Antlejos Terraforming Common Functions

--]]
local vn = require "vn"
local mt = require 'merge_tables'

local antlejos = {}

antlejos.verner = {
   --portrait = "verner.webp",
   --image = "verner.webp",
   portrait = "nelly.webp",
   image = "nelly.webp",
   name = _("Verner"),
   color = nil,
   transition = nil, -- Use default
}

function antlejos.vn_verner( params )
   return vn.Character.new( antlejos.verner.name,
         mt.merge_tables( {
            image=antlejos.verner.image,
            color=antlejos.verner.colour,
         }, params) )
end

-- Function for adding log entries for miscellaneous one-off missions.
function antlejos.log( text )
   shiplog.create( "antlejos", _("Antlejos V"), _("Neutral") )
   shiplog.append( "antlejos", text )
end

antlejos.rewards = {
   ant01 = 150e3,
   ant02 = 200e3,
   ant03 = 250e3,
   ant04 = 250e3,
   ant05 = 350e3,
   ant06 = 200e3, -- Repeatable
}

return antlejos

