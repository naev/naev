--[[
   Mission helper stuff
--]]
local audio = require 'love.audio'
local lmisn = {}
      
local _sfx = {
   victory = audio.newSource( 'snd/sounds/jingles/victory.ogg' ),
}

function lmisn.sfxVictory ()
   local sfx = _sfx.victory:clone()
   sfx:play()
end

function lmisn.sfxMoney ()
end

return lmisn
