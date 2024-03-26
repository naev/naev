--[[
   Common stuff for the Onion Society campaign
--]]
local love_shaders = require "love_shaders"
local vn = require "vn"
local lg = require "love.graphics"

local onion = {}

local img_onion
function onion.img_onion ()
   if not img_onion then
      img_onion = lg.newImage( "gfx/misc/onion_society.webp" )
   end
   return img_onion
end

onion.loops = {
   circus = "snd/sounds/loops/onion_circus.ogg",
   hacker = "snd/sounds/loops/onion_hacker.ogg",
}

function onion.vn_onion( params )
   return vn.Character.new( _("Hologram"),
         tmerge( {
            image=onion.img_onion(),
            colour=nil,
            shader=love_shaders.hologram{strength=0.2},
         }, params) )
end

function onion.vn_l337b01( params )
   return vn.Character.new( _("l337 b01"),
         tmerge( {
            image=onion.img_onion(), -- TODO
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
   misn02 = 500e3,
   misn03 = 700e3,
}

return onion
