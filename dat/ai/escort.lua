require 'ai.core.core'
require 'ai.core.control.escort'

-- Do not distress or board
mem.distress   = false
mem.atk_board  = false
-- Some defaults that should get overwritten
mem.aggressive = true
mem.enemyclose = 3e3
mem.leadermaxdist = 8e3
mem.atk_kill   = false
