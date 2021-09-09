--[[
   Mission helper stuff
--]]
local lmisn = {}
      
local _sfx_victory = audio.newSource( 'snd/sounds/jingles/victory.ogg' ),

function lmisn.sfxVictory ()
   local sfx = _sfx_victory:clone()
   sfx:play()
end

function lmisn.sfxMoney ()
end

return lmisn
