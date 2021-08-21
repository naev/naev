require 'ai.core.core'
require 'ai.core.idle.miner'
require 'ai.core.misc.distress'
require "numstring"

mem.lanes_useneutral = true

function create ()
   ai.setcredits( rnd.rnd(ai.pilot():ship():price()/500, ai.pilot():ship():price()/200) )

   -- Communication stuff
   mem.bribe_no = _([["I don't want any problems."]])

   create_post()
end

function hail ()
   -- Refuel
   if mem.refuel == nil then
      mem.refuel = rnd.rnd( 1000, 3000 )
      mem.refuel_msg = string.format(_([["I'll supply your ship with fuel for %s."]]),
            creditstring(mem.refuel))
   end
end
