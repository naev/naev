require 'ai.core.core'
require 'ai.core.idle.civilian'
require 'ai.core.misc.distress'
require "numstring"

local bribe_no_list = {
   _([["Just leave me alone!"]]),
   _([["What do you want from me!?"]]),
   _([["Get away from me!"]])
}

function create ()
   -- Credits.
   ai.setcredits( rnd.rnd(ai.pilot():ship():price()/500, ai.pilot():ship():price()/200) )

   -- No bribe
   mem.bribe_no = bribe_no_list[ rnd.rnd(1,#bribe_no_list) ]

   -- Refuel
   mem.refuel = rnd.rnd( 1000, 3000 )
   local p = player.pilot()
   if p:exists() then
      standing = ai.getstanding( p ) or -1
      mem.refuel_msg = string.format(_([["I'll supply your ship with fuel for %s."]]),
            creditstring(mem.refuel))
   end

   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system
   create_post()
end

