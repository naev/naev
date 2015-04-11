--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--]]

include("dat/ai/tpl/generic.lua")
include("dat/ai/personality/patrol.lua")

-- Settings
mem.aggressive = true


-- Create function
function create ()

   -- Credits
   ai.setcredits( rnd.int(ai.shipprice()/300, ai.shipprice()/70) )

   -- Bribing
   bribe_no = {
         "\"You insult my honour.\"",
         "\"I find your lack of honour disturbing.\"",
         "\"You disgust me.\"",
         "\"Bribery carries a harsh penalty.\"",
         "\"House Goddard does not lower itself to common scum.\""
   }
   mem.bribe_no = bribe_no[ rnd.rnd(1,#bribe_no) ]

   -- Refueling
   p = ai.getPlayer()
   if ai.exists(p) then
      standing = ai.getstanding( p ) or -1
      mem.refuel = rnd.rnd( 2000, 4000 )
      if standing > 60 then mem.refuel = mem.refuel * 0.7 end
      mem.refuel_msg = string.format( "\"I could do you the favour of refueling for the price of %d credits.\"",
            mem.refuel )
   end

   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system

   -- Finish up creation
   create_post()
end

-- taunts
function taunt ( target, offense )
   -- Offense is not actually used
   taunts = {
         "Prepare to face annihilation!",
         "I shall wash my hull in your blood!",
         "Your head will make a great trophy!",
         "These moments will be your last!",
         "You are a parasite!"
   }
   ai.comm( target, taunts[ rnd.int(1,#taunts) ] )
end

