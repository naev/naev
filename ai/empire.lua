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
function taunt ( target, offense )

   -- Only 50% of actually taunting.
   if rnd.int(0,1) == 0 then
      return
   end

   -- some taunts
   if offense then
      taunts = {
            "There is no room in this universe for scum like you!",
            "The Empire will enjoy your death!",
            "Your head will make a fine gift for the Emperor!"
      }
   else
      taunts = {
            "You dare attack me!",
            "You are no match for the Empire!",
            "The Empire will have your head!",
            "You'll regret this!"
      }
   end

   ai.comm(target, taunts[ rnd.int(1,#taunts) ])
end


