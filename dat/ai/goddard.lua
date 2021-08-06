require 'ai.core.core'
require "numstring"

-- Settings
mem.aggressive = true

local bribe_no_list = {
   _([["You insult my honour."]]),
   _([["I find your lack of honour disturbing."]]),
   _([["You disgust me."]]),
   _([["Bribery carries a harsh penalty."]]),
   _([["House Goddard does not lower itself to common scum."]])
}
local taunt_list = {
   _("Prepare to face annihilation!"),
   _("I shall wash my hull in your blood!"),
   _("Your head will make a great trophy!"),
   _("These moments will be your last!"),
   _("You are a parasite!")
}

-- Create function
function create ()

   -- Credits
   ai.setcredits( rnd.rnd(ai.pilot():ship():price()/300, ai.pilot():ship():price()/70) )

   -- Refueling
   local p = player.pilot()
   if p:exists() then
      local standing = ai.getstanding( p ) or -1
      mem.refuel = rnd.rnd( 2000, 4000 )
      if standing > 60 then mem.refuel = mem.refuel * 0.7 end
      mem.refuel_msg = string.format( _([["I could do you the favour of refueling for the price of %s."]]),
            creditstring(mem.refuel) )
   end

   -- Bribing
   mem.bribe_no = bribe_no_list[ rnd.rnd(1,#bribe_no_list) ]

   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system

   -- Finish up creation
   create_post()
end

-- taunts
function taunt ( target, offense )
   -- Offense is not actually used
   local taunts = taunt_list
   ai.pilot():comm( target, taunts[ rnd.rnd(1,#taunts) ] )
end

