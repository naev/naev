include("dat/ai/tpl/generic.lua")
include("dat/ai/personality/patrol.lua")

-- Settings
mem.armour_run = 40
mem.armour_return = 70
mem.aggressive = true


function create ()

   ai.setcredits( rnd.int(ai.shipprice()/150, ai.shipprice()/50) )

   if rnd.int() > 0.7 then
      mem.bribe = math.sqrt( ai.shipmass() ) * (750. * rnd.int() + 2500.)
      mem.bribe_prompt = string.format("\"Your life is worth %d credits to me.\"", mem.bribe )
      mem.bribe_paid = "\"Beat it.\""
   else
      if rnd.int() > 0.5 then
         mem.bribe_no = "\"You won't buy your way out of this one.\""
      else
         mem.bribe_no = "\"I'm afraid you can't make it worth my while.\""
      end
   end

   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system

   -- Finish up creation
   create_post()
end

-- taunts
function taunt ( target, offense )

   -- Only 20% of actually taunting.
   if rnd.int(0,4) ~= 0 then
      return
   end

   -- some taunts
   if offense then
      taunts = {
            "Don't take this personally.",
            "It's just business."
      }
   else
      taunts = {
            "Your skull will make a great hood ornament.",
            "I've destroyed ships twice the size of yours!",
            "I'll crush you like a grape!",
            "This isn't what I signed up for!"
      }
   end

   ai.comm(target, taunts[ rnd.int(1,#taunts) ])
end


