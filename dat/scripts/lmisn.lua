--[[
   Mission helper stuff
--]]
local lmisn = {}
      
local audio
local _sfx
local function _sfx_load ()
   audio = = require 'love.audio'
   _sfx = {
      victory = audio.newSource( 'snd/sounds/jingles/victory.ogg' ),
   }
end

function lmisn.sfxVictory ()
   if not _sfx then _sfx_load() end

   local sfx = _sfx.victory:clone()
   sfx:play()
end

function lmisn.sfxMoney ()
end

return lmisn
