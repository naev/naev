require 'ai.core.core'

-- Settings
mem.shield_run    = -1
mem.armour_run    = -1
mem.norun         = true
mem.aggressive    = true
mem.lanes_useneutral = false
mem.bribe_no      = _([[No response.]])
mem.refuel_no     = _([[No response.]])

function create ()
   create_pre()

   local price = ai.pilot():ship():price()
   ai.setcredits( rnd.rnd(price/80, price/30) )

   -- Late game challenge
   mem.atk_skill = 1.0

   -- Finish up creation
   create_post()
end
