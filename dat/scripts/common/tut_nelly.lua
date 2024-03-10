--[[

   Common stuff for the Nelly tutorial campaign

--]]
local vn = require "vn"

local tutnel = {}

tutnel.nelly = {
   portrait = "nelly.webp",
   image = "nelly.webp",
   name = _("Nelly"),
   colour = nil,
   transition = nil, -- Use default
}

function tutnel.vn_nelly( params )
   return vn.Character.new( tutnel.nelly.name,
         tmerge( {
            image=tutnel.nelly.image,
            colour=tutnel.nelly.colour,
         }, params) )
end

function tutnel.log( text )
   shiplog.create( "tut_nelly", _("Nelly"), _("Neutral") )
   shiplog.append( "tut_nelly", text )
end

tutnel.reward = {
   nelly01 = 80e3,
   nelly02 = 120e3,
   nelly03 = 57928, -- Spare change
}

return tutnel
