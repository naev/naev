require 'ai.core.core'

mem.aggressive = true

-- No idle function, ship won't react until it detects enemies
-- luacheck: globals idle (AI Task functions passed by name)
function idle ()
end
