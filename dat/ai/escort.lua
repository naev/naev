require 'ai.core.core'
require 'ai.core.control.escort'

-- Do not distress or board
mem.distress   = false
-- TODO the options below here should be player-configurable
mem.aggressive = true
mem.enemyclose = 3e3
mem.leadermaxdist = 8e3
mem.atk_kill   = false
