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
      img_onion = lg.newImage( "gfx/misc/onion_society" )
   end
   return img_onion
end

function onion.img_l337b01 ()
   return lg.newImage( "gfx/vn/characters/l337_b01" )
end

function onion.img_trixie ()
   local img = lg.newImage( "gfx/vn/characters/trixie" )
   img:setFilter( "nearest", "nearest" )
   return img
end

onion.loops = {
   circus = "snd/sounds/loops/onion_circus.ogg",
   hacker = "snd/sounds/loops/onion_hacker.ogg",
}

onion.npc = {
   l337b01 = {
      name = _("l337 b01"),
      image = "l337_b01",
      colour = {0.05, 1.0, 0.6}, -- Teal-ish
   },
   trixie = {
      name = _("Trixie"),
      colour = {0.7,0.1,0.9}, -- Dark Purple
   },
   dog = {
      name = _("DOG"),
      image = "DOG",
      colour = {0.6,0.33,0.0}, -- Dark Brown
   },
   notasockpuppet = {
      name = _("notasockpuppet"),
      image = "notasockpuppet",
      colour = {1.0, 0.2, 0.9}, -- Light Purple
   },
   underworlder = {
      name = _("underworlder"),
      image = "underworlder",
      colour = {0.9, 0.7, 0.25},
   },
   lonewolf4 = {
      name = _("lonewolf4"),
      image = "lonewolf4",
      colour = {0.7, 0.7, 0.7},
   }
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
   params = params or {}
   params.shader = love_shaders.hologram{strength=0.2}
   return onion.vn_nexus_l337b01( params )
end

function onion.vn_trixie( params )
   params = params or {}
   params.shader = love_shaders.hologram{strength=0.2}
   return onion.vn_nexus_trixie( params )
end

function onion.vn_nexus_l337b01( params )
   return vn.Character.new( onion.npc.l337b01.name,
         tmerge( {
            image=onion.npc.l337b01.image,
            colour=onion.npc.l337b01.colour,
            flip=false,
         }, params) )
end

function onion.vn_nexus_trixie( params )
   return vn.Character.new( onion.npc.trixie.name,
         tmerge( {
            image=onion.img_trixie(),
            colour=onion.npc.trixie.colour,
         }, params) )
end

function onion.vn_nexus_underworlder( params )
   return vn.Character.new( onion.npc.underworlder.name,
         tmerge( {
            image=onion.npc.underworlder.image,
            colour=onion.npc.underworlder.colour,
         }, params) )
end

function onion.vn_nexus_notasockpuppet( params )
   return vn.Character.new( onion.npc.notasockpuppet.name,
         tmerge( {
            image=onion.npc.notasockpuppet.image,
            colour=onion.npc.notasockpuppet.colour,
         }, params) )
end

function onion.vn_dog( params )
   params = params or {}
   params.shader = love_shaders.hologram{strength=0.2}
   return onion.vn_nexus_dog( params )
end

function onion.vn_nexus_dog( params )
   return vn.Character.new( onion.npc.dog.name,
         tmerge( {
            image=onion.npc.dog.image,
            colour=onion.npc.dog.colour,
         }, params) )
end

function onion.vn_lonewolf4( params )
   params = params or {}
   params.shader = love_shaders.hologram{strength=0.2}
   return onion.vn_nexus_lonewolf4( params )
end

function onion.vn_nexus_lonewolf4( params )
   return vn.Character.new( onion.npc.lonewolf4.name,
         tmerge( {
            image=onion.npc.lonewolf4.image,
            colour=onion.npc.lonewolf4.colour,
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
   misn04 = 800e3,
   misn07 = 500e3,
   misn08 = 1e6,
}

return onion
