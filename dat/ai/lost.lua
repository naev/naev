require 'ai.core.core'

-- Settings
mem.aggressive     = true
mem.safe_distance  = 2000
mem.armour_run     = 10
mem.armour_return  = 30
mem.atk_kill       = true
mem.atk_board      = false
local msg = _([[You only hear static.]])
mem.bribe_no       = msg
mem.refuel_no      = msg
mem.comm_no        = msg

function create ()
   create_pre()
   create_post()
end
