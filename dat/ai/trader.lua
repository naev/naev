require 'ai.core.core'
require 'ai.core.idle.trader'
--require 'ai.core.control.trader'
require 'ai.core.misc.distress'

-- Always run away
mem.aggressive    = false
mem.formation     = "buffer"
mem.lanes_useneutral = true
mem.atk_skill = 0

function create ()
   create_pre()

   local p = ai.pilot()
   local ps = p:ship()
   -- Probably the ones with the most money
   local price = ps:price()
   ai.setcredits( rnd.rnd(price/100, price/25) )

   -- Try to do normal life as much as possible
   mem.safe_distance = 2000 + 1000 * ps:size()

   -- A bit more skilled if not a transport
   if not ps:tags().transport then
      mem.atk_skill  = 0.5 + 0.3*rnd.sigma()
   end

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

   -- Deal with refuelling
   local standing = p:reputation()
   mem.refuel = mem.refuel_base
   if standing > 50 then
      mem.refuel = mem.refuel * 0.75
   elseif standing > 80 then
      mem.refuel = mem.refuel * 0.5
   end
   mem.refuel_msg = _([["I'll supply your ship with fuel for {credits}."]])

   -- Bribes
   mem.bribe_no = _([["The Space Traders do not negotiate with criminals."]])
end
