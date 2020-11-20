require("ai/tpl/generic.lua")
require("ai/personality/trader.lua")
require("ai/distress_behaviour.lua")
require "numstring.lua"


function create ()

   -- Probably the ones with the most money
   ai.setcredits( rnd.int(ai.pilot():ship():price()/100, ai.pilot():ship():price()/25) )

   -- Communication stuff
   mem.bribe_no = _("\"The Space Traders do not negotiate with criminals.\"")
   mem.refuel = rnd.rnd( 3000, 5000 )
   p = player.pilot()
   if p:exists() then
      standing = ai.getstanding( p ) or -1
      if standing > 50 then mem.refuel = mem.refuel * 0.75
      elseif standing > 80 then mem.refuel = mem.refuel * 0.5
      end
      mem.refuel_msg = string.format(_("\"I'll supply your ship with fuel for %s.\""),
            creditstring(mem.refuel));
   end

   -- Some stuff has more chance then others
   num = rnd.int(12)
   if num < 5 then
      cargo = "Food"
   elseif num < 8 then
      cargo = "Ore"
   elseif num < 10 then
      cargo = "Industrial Goods"
   elseif num < 12 then
      cargo = "Luxury Goods"
   else
      cargo = "Medicine"
   end
   ai.pilot():cargoAdd( cargo, rnd.int(0, ai.pilot():cargoFree() ) )

   -- Finish up creation
   create_post()
end
