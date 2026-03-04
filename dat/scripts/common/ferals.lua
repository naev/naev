--[[

   Feral Bioship Common Functions

--]]
local audio = require "love.audio"

local ferals = {}

ferals.sfx = {
   spacewhale1 = audio.newSoundData( "snd/sounds/spacewhale1" ),
   spacewhale2 = audio.newSoundData( "snd/sounds/spacewhale2" ),
   bite = audio.newSoundData( "snd/sounds/crash1" ),
}

function ferals.faction ()
   local id = "feralbioship"
   local f = faction.exists( id )
   if f then
      return f
   end
   return faction.dynAdd( nil, id, _("Feral Bioship"), {ai="feralbioship"} )
end

return ferals
