require 'ai.core.core'
require 'ai.core.idle.trader'
--require 'ai.core.control.trader'
require 'ai.core.misc.distress'
require "numstring"

-- Always run away
mem.aggressive = false
mem.lanes_useneutral = true

function create ()
   local p = ai.pilot()
   local ps = p:ship()
   -- Probably the ones with the most money
   local price = ps:price()
   ai.setcredits( rnd.rnd(price/100, price/25) )

   -- Try to do normal life as muchas possible
   mem.safe_distance = 2000 + 500 * ps:size()

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
