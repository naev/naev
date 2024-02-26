--[[
   Common stuff for the Onion Society campaign
--]]
local love_shaders = require "love_shaders"
local vn = require "vn"

local onion = {}

function onion.vn_onion( params )
   return vn.Character.new( _("Hologram"),
         tmerge( {
            image=nil,
            colour=nil,
            shader=love_shaders.hologram{strength=0.2},
         }, params) )
end

return onion
