require 'ai.core.core'
require 'ai.core.idle.miner'
require 'ai.core.misc.distress'
require "numstring"

mem.lanes_useneutral = true

function create ()
   ai.setcredits( rnd.rnd(ai.pilot():ship():price()/500, ai.pilot():ship():price()/200) )

   hail()

   create_post()
end

function hail ()
   local p = ai.pilot()

   -- Remove randomness from future calls
   if not mem.hailsetup then
      mem.refuel_base = rnd.rnd( 1000, 3000 )
      mem.hailsetup = true
   end

   -- Clean up
   mem.refuel        = 0
   mem.refuel_msg    = nil
   mem.bribe         = 0
   mem.bribe_prompt  = nil
   mem.bribe_prompt_nearby = nil
   mem.bribe_paid    = nil
   mem.bribe_no      = nil

   -- Refuel
   mem.refuel = mem.refuel_base
   mem.refuel_msg = string.format(_([["I'll supply your ship with fuel for %s."]]),
         creditstring(mem.refuel))

   -- Communication stuff
   mem.bribe_no = _([["I don't want any problems."]])
end
