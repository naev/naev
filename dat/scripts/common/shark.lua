--[[

   Shark Common Functions

--]]
local portrait = require "portrait"
local vn = require "vn"

local shark = {}

function shark.pirateFaction ()
   local enemy = faction.dynAdd( "Pirate", "Wanted Pirate", _("Wanted Pirate"), {clear_enemies=true, clear_allies=true} )
   return enemy
end

function shark.addLog( text )
   shiplog.create( "shark", _("Nexus Shipyards"), _("Nexus Shipyards") )
   shiplog.append( "shark", text )
end

shark.arnold = {
   name = _("Arnold Smith"),
   portrait = "neutral/unique/arnoldsmith.webp",
   colour = nil,
   transition = "pixelize",
   description = _(""),
}
shark.arnold.image = portrait.getFullPath(shark.arnold.portrait)

function shark.vn_arnold( params )
   return vn.Character.new( shark.arnold.name,
      tmerge( {
         image=shark.arnold.image,
         color=shark.arnold.colour,
      }, params) )
end

shark.rewards = {
   sh00 = 500e3,
   sh01 = 750e3,
   sh02 = 750e3,
   sh03 = 750e3,
   sh04 = 750e3,
   sh05 = 1e6,
   sh06 = 750e3,
   sh07 = 3e6, -- + "Sandwich Holder" accessory
}

return shark
