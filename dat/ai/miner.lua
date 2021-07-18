require 'ai.core.core'
require 'ai.core.idle.miner'
require 'ai.core.misc.distress'
require "numstring"

function create ()

   ai.setcredits( rnd.int(ai.pilot():ship():price()/500, ai.pilot():ship():price()/200) )

   -- Communication stuff
   mem.bribe_no = _("\"I don't want any problem.\"")

   -- Refuel
   mem.refuel = rnd.rnd( 1000, 3000 )
   local p = player.pilot()
   if p:exists() then
      mem.refuel_msg = string.format(_("\"I'll supply your ship with fuel for %s.\""),
            creditstring(mem.refuel))
   end

   create_post()
end
