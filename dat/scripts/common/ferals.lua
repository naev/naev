--[[

   Feral Bioship Common Functions

--]]
local audio = require "love.audio"

local ferals = {}

ferals.sfx = {
   spacewhale1 = audio.newSource( "snd/sounds/spacewhale1.ogg" ),
   spacewhale2 = audio.newSource( "snd/sounds/spacewhale2.ogg" ),
   bite = audio.newSource( "snd/sounds/crash1.ogg" ),
}

return ferals
