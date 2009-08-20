include("ai/tpl/generic.lua")

-- Settings
mem.aggressive = true


-- Create function
function create ()

   -- Credits.
   ai.setcredits( rnd.int(ai.shipprice()/500, ai.shipprice()/200) )

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

   -- Get attack function.
   attack_choose()
end

-- taunts
function taunt ( target, offense )
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

