require 'ai.core.core'
require 'ai.core.idle.civilian'
require 'ai.core.misc.distress'

mem.lanes_useneutral = true
mem.atk_skill = 0

local bribe_no_list = {
   _([["Just leave me alone!"]]),
   _([["What do you want from me!?"]]),
   _([["Get away from me!"]])
}

function create ()
   create_pre()
   -- Credits.
   ai.setcredits( rnd.rnd(ai.pilot():ship():price()/500, ai.pilot():ship():price()/200) )

   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system
   create_post()
end


function hail ()
   if mem.setuphail then return end

   -- No bribe
   mem.bribe_no = bribe_no_list[ rnd.rnd(1,#bribe_no_list) ]

   -- Refueling
   mem.refuel = rnd.rnd( 1000, 3000 )
   mem.refuel_msg = _([["I'll supply your ship with fuel for {credits}."]])

   mem.setuphail = true
end
