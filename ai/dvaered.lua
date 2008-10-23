include("ai/tpl/generic.lua")

-- Settings
aggressive = true


-- Create function
function create ()
   ai.setcredits( rnd.int(1000, ai.shipprice()/200) )
   attack_choose()
   if rnd.int() > 0.4 then
      mem.bribe_no = "\"I shall especially enjoy your death.\""
   else
      mem.bribe_no = "\"You shall not buy my honour!\""
   end
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

