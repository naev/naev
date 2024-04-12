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

function onion.img_l337b01 ()
   return lg.newImage( "gfx/vn/characters/l337_b01.webp" )
end

function onion.img_trixie ()
   local img = lg.newImage( "gfx/vn/characters/trixie.webp" )
   img:setFilter( "nearest", "nearest" )
   return img
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
            image="l337_b01.webp",
            colour=nil,
            shader=love_shaders.hologram{strength=0.2},
            flip=false,
         }, params) )
end

function onion.vn_nexus_l337b01( params )
   return vn.Character.new( _("l337 b01"),
         tmerge( {
            image="l337_b01.webp",
            colour=nil,
            flip=false,
         }, params) )
end

function onion.vn_nexus_trixie( params )
   return vn.Character.new( _("Trixie"),
         tmerge( {
            image=onion.img_trixie(),
            colour=nil,
         }, params) )
end

function onion.vn_nexus_underworlder( params )
   return vn.Character.new( _("underworlder"),
         tmerge( {
            image=onion.img_onion(), -- TODO
            colour=nil,
         }, params) )
end

function onion.vn_nexus_notasockpuppet( params )
   return vn.Character.new( _("notasockpuppet"),
         tmerge( {
            image="notasockpuppet.webp",
            colour=nil,
         }, params) )
end

function onion.vn_nexus_dog( params )
   return vn.Character.new( _("DOG"),
         tmerge( {
            image="DOG.webp",
            colour=nil,
         }, params) )
end

function onion.vn_nexus_lonewolf4( params )
   return vn.Character.new( _("lonewolf4"),
         tmerge( {
            image=onion.img_onion(), -- TODO
            colour=nil,
         }, params) )
end

function onion.log( text )
   shiplog.create( "onion", _("Onion Society"), _("Onion Society") )
   shiplog.append( "onion", text )
end

onion.rewards = {
   misn01 = 200e3,
   misn02 = 400e3,
   misn03 = 600e3,
}

return onion
