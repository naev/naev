require("ai/tpl/generic")
require("ai/personality/miner")
require("ai/distress_behaviour")
require "numstring"


function create ()

   ai.setcredits( rnd.int(ai.pilot():ship():price()/500, ai.pilot():ship():price()/200) )

   -- Communication stuff
   mem.bribe_no = _("\"I don't want any problem.\"")

   -- Refuel
   mem.refuel = rnd.rnd( 1000, 3000 )
   p = player.pilot()
   if p:exists() then
      standing = ai.getstanding( p ) or -1
      mem.refuel_msg = string.format(_("\"I'll supply your ship with fuel for %s.\""),
            creditstring(mem.refuel));
   end

   create_post()
end
