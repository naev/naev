require 'ai.core.core'
require "numstring"


mem.shield_run = 100
mem.armour_run = 100
mem.defensive  = false
mem.enemyclose = 500
mem.careful   = true


function create ()
   -- Credits.
   ai.setcredits( rnd.rnd(ai.pilot():ship():price()/500, ai.pilot():ship():price()/200) )

   -- No bribe
   local bribe_msg = {
      _("\"Just leave me alone!\""),
      _("\"What do you want from me!?\""),
      _("\"Get away from me!\"")
   }
   mem.bribe_no = bribe_msg[ rnd.rnd(1,#bribe_msg) ]

   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system
   create_post()
end


function hail ()
   -- Refuel
   if mem.refuel == nil then
      mem.refuel = rnd.rnd( 1000, 3000 )
      mem.refuel_msg = string.format(_("\"I'll supply your ship with fuel for %s.\""),
            creditstring(mem.refuel))
   end
end
