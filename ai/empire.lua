include("ai/tpl/generic.lua")

-- Settings
armour_run = 40
armour_return = 70
aggressive = true


function create ()

   ai.setcredits( rnd.int(ai.shipprice()/300, ai.shipprice()/70) )

   if rnd.int(0,2)==0 then
      ai.broadcast("The Empire is watching you.")
   end

   if rnd.int() > 0.7 then
      mem.bribe = math.sqrt( ai.shipmass() ) * (500. * rnd.int() + 1750.)
      mem.bribe_prompt = string.format("\"For some %d credits I could forget about seeing you.\"", mem.bribe )
      mem.bribe_paid = "\"Now scram before I change my mind.\""
   else
      if rnd.int() > 0.5 then
         mem.bribe_no = "\"You won't buy your way out of this one.\""
      else
         mem.bribe_no = "\"The Empire likes to make examples out of scum like you.\""
      end
   end

   attack_choose()
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


