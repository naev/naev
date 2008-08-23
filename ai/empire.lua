include("ai/tpl/generic.lua")

-- Settings
armour_run = 40
armour_return = 70
aggressive = true


function create ()

   ai.setcredits( rnd.int(1000, ai.shipprice()/70) )

   if rnd.int(0,2)==0 then
      ai.broadcast("The Empire is watching you.")
   end
end

-- taunts
function taunt ( target )
   taunts = {
         "You dare attack me!",
         "You are no match for the Empire!",
         "The Empire will have your head!",
         "You'll regret this!"
   }
   ai.comm(target, taunts[ rnd.int(1,#taunts) ])
end

