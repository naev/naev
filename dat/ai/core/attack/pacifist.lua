--[[
--    Pacifist attack functions
--]]
local atk_pacifist = {}

function atk_pacifist.think( _target, _si )
   ai.poptask()
   return
end

function atk_pacifist.attacked( _attacker )
   return
end

function atk_pacifist.atk( _target, _dokill )
   ai.poptask()
   return
end

return atk_pacifist
