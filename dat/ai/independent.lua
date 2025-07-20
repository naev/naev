require 'ai.core.core'
require 'ai.core.idle.civilian'
require 'ai.core.misc.distress'

mem.careful   = false
mem.lanes_useneutral = true

local bribe_msg_list = {
   _([["Just leave me alone!"]]),
   _([["What do you want from me!?"]]),
   _([["Get away from me!"]])
}

function create ()
   create_pre()

   -- Credits.
   local price = ai.pilot():ship():price()
   ai.setcredits( rnd.rnd(price/150, price/50) ) -- Target for crime

   mem.atk_skill = 0.3 + 0.3*rnd.sigma()

   create_post()
end

function hail ()
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

   mem.refuel = mem.refuel_base
   mem.refuel_msg = _([["I'll supply your ship with fuel for {credits}."]])

   -- No bribe
   mem.bribe_no = bribe_msg_list[ rnd.rnd(1,#bribe_msg_list) ]
end
