include("ai/tpl/generic.lua")

-- Settings
aggressive = true


-- Create function
function create ()

   -- Credits
   ai.setcredits( rnd.int(ai.shipprice()/300, ai.shipprice()/70) )

   -- Bribing
   mem.bribe_no = "\"Be gone!\""

   -- Refueling
   p = ai.getPlayer()
   if ai.exists(p) then
      standing = ai.getstanding( p ) or -1
      mem.refuel = rnd.rnd( 2000, 4000 )
      if standing > 60 then mem.refuel = mem.refuel * 0.7 end
      mem.refuel_msg = string.format( "\"I could do you the favour of refueling for the price of %d credits.\"",
            mem.refuel )
   end

   -- Get attack
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

