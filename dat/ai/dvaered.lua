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

   -- Credits.
   ai.setcredits( rnd.int(ai.shipprice()/300, ai.shipprice()/100) )

   -- Handle bribing
   if rnd.int() > 0.4 then
      mem.bribe_no = "\"I shall especially enjoy your death.\""
   else
      bribe_no = {
            "\"You insult my honour.\"",
            "\"I find your lack of honour disturbing.\"",
            "\"You disgust me.\"",
            "\"Bribery carries a harsh penalty.\"",
            "\"House Dvaered does not lower itself to common scum.\""
     }
     mem.bribe_no = bribe_no[ rnd.rnd(1,#bribe_no) ]
   end

   -- Handle refueling
   p = ai.getPlayer()
   if ai.exists(p) then
      standing = ai.getstanding( p ) or -1
      mem.refuel = rnd.rnd( 1000, 3000 )
      if standing < 50 then
         mem.refuel_no = "\"You are not worthy of my attention.\""
      else
         mem.refuel_msg = string.format("\"For you I could make an exception for %d credits.\"", mem.refuel)
      end
   end

   -- Handle misc stuff
   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system

   create_post()
end

-- taunts
function taunt ( target, offense )

   -- Only 50% of actually taunting.
   if rnd.rnd(0,1) == 0 then
      return
   end

   -- Offense is not actually used
   taunts = {
       "Prepare to face annihilation!",
       "I shall wash my hull in your blood!",
       "Your head will make a great trophy!",
       "You're no match for the Dvaered!",
       "Death awaits you!"
   }
   ai.comm( target, taunts[ rnd.int(1,#taunts) ] )
end

