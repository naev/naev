--[[

   Common stuff for the Nelly tutorial campaign

--]]
local vn = require "vn"
local mt = require 'merge_tables'

local tutnel = {}

-- TODO replace with real portrait
tutnel.nelly = {
   portrait = "nelly.webp",
   image = "nelly.webp",
   name = _("Nelly"),
   color = nil,
   transition = nil, -- Use default
}

function tutnel.vn_nelly( params )
   return vn.Character.new( tutnel.nelly.name,
         mt.merge_tables( {
            image=tutnel.nelly.image,
            color=tutnel.nelly.colour,
         }, params) )
end

function tutnel.log( text )
   shiplog.create( "tut_nelly", _("Nelly"), _("Neutral") )
   shiplog.append( "tut_nelly", text )
end

return tutnel
