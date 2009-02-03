include("ai/tpl/generic.lua")

-- Settings
aggressive = true


-- Create function
function create ()
   ai.setcredits( rnd.int(ai.shipprice()/300, ai.shipprice()/70) )
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

