include("ai/tpl/generic.lua")

-- Settings
aggressive = true
safe_distance = 500
armour_run = 80
armour_return = 100


function create ()
   ai.setcredits(ai.shipprice()/1000 , ai.shipprice()/100 )
   mem.bribe = math.sqrt( ai.shipmass() ) * (300. * rnd.int() + 850.)
   attack_choose()
end


function taunt ( target, offense )

   -- Only 50% of actually taunting.
   if rnd.int(0,1) == 0 then
      return
   end

   -- some taunts
   if offense then
      taunts = {
            "Prepare to be boarded!",
            "Yohoho!",
            "What's a ship like you doing in a place like this?"
      }
   else
      taunts = {
            "You dare attack me!",
            "You think that you can take me on?",
            "Die!",
            "You'll regret this!"
      }
   end

   ai.comm(target, taunts[ rnd.int(1,#taunts) ])
end

