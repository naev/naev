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

function onion.log( text )
   shiplog.create( "onion", _("Onion Society"), _("Onion Society") )
   shiplog.append( "onion", text )
end

onion.rewards = {
   misn01 = 200e3,
}

return onion
