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

antlejos.unidiff_list = {
   "antlejosv_1",
   "antlejosv_2",
   "antlejosv_3",
   "antlejosv_4",
}

function antlejos.unidiff( diffname )
   for _k,d in ipairs(antlejos.unidiff_list) do
      if diff.isApplied(d) then
         diff.remove(d)
      end
   end
   diff.apply( diffname )
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

