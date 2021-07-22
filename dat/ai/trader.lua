require 'ai.core.core'
require 'ai.core.idle.trader'
--require 'ai.core.control.trader'
require 'ai.core.misc.distress'
require "numstring"

-- Always run away
mem.aggressive = false

function create ()

   -- Probably the ones with the most money
   ai.setcredits( rnd.int(ai.pilot():ship():price()/100, ai.pilot():ship():price()/25) )

   -- Communication stuff
   mem.bribe_no = _("\"The Space Traders do not negotiate with criminals.\"")
   mem.refuel = rnd.rnd( 3000, 5000 )
   local p = player.pilot()
   if p:exists() then
      local standing = ai.getstanding( p ) or -1
      if standing > 50 then
         mem.refuel = mem.refuel * 0.75
      elseif standing > 80 then
         mem.refuel = mem.refuel * 0.5
      end
      mem.refuel_msg = string.format(_("\"I'll supply your ship with fuel for %s.\""),
            creditstring(mem.refuel))
   end

   -- Finish up creation
   create_post()
end
