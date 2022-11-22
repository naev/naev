require 'ai.core.core'
mem.aggressive = true -- Player is an asshole
mem.atk_kill = false -- Should propagate to fighters

function should_attack( _enemy, _si, _aggressor )
   return true -- should work better with escorts
end
