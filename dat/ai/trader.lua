require 'ai.core.core'
require 'ai.core.idle.trader'
--require 'ai.core.control.trader'
require 'ai.core.misc.distress'
require "numstring"

-- Always run away
mem.aggressive = false
mem.lanes_useneutral = true

function create ()
   -- Probably the ones with the most money
   ai.setcredits( rnd.rnd(ai.pilot():ship():price()/100, ai.pilot():ship():price()/25) )

   hail()

   -- Finish up creation
   create_post()
end

function hail ()
   local p = ai.pilot()

   -- Remove randomness from future calls
   if not mem.hailsetup then
      mem.refuel_base = rnd.rnd( 3000, 5000 )
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

   -- Deal with refueling
   local standing = p:faction():playerStanding()
   mem.refuel = mem.refuel_base
   if standing > 50 then
      mem.refuel = mem.refuel * 0.75
   elseif standing > 80 then
      mem.refuel = mem.refuel * 0.5
   end
   mem.refuel_msg = string.format(_([["I'll supply your ship with fuel for %s."]]),
         creditstring(mem.refuel))

   -- Bribes
   mem.bribe_no = _([["The Space Traders do not negotiate with criminals."]])
end
