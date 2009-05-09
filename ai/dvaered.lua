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
      mem.bribe_no = "\"You shall not buy my honour!\""
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
         "Your head will make a great trophy!"
   }
   ai.comm( target, taunts[ rnd.int(1,#taunts) ] )
end

