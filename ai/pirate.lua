include("ai/tpl/generic.lua")

--[[

   Pirate AI

--]]

-- Settings
mem.aggressive     = true
mem.safe_distance  = 500
mem.armour_run     = 80
mem.armour_return  = 100
mem.atk_board      = true
mem.atk_kill       = false


function create ()

   -- Some pirates do kill
   if rnd.rnd() < 0.7 then
      mem.atk_kill = true
   end

   -- Not too much money
   ai.setcredits(ai.shipprice()/80 , ai.shipprice()/30 )

   -- Deal with bribeability
   if rnd.rnd() < 0.05 then
      mem.bribe_no = "\"You won't be able to slide out of this one!\""
   else
      mem.bribe = math.sqrt( ai.shipmass() ) * (300. * rnd.rnd() + 850.)
      if rnd.rnd() > 0.5 then
         mem.bribe_prompt = string.format("\"It'll cost you %d credits for me to ignore your pile of rubbish.\"", mem.bribe)
         mem.bribe_paid = "\"You're lucky I'm so kind.\""
      else
         mem.bribe_prompt = string.format("\"I'm in a good mood so I'll let you go for %d credits.\"", mem.bribe)
         mem.bribe_paid = "\"Life doesn't get easier then this.\""
      end
   end

   -- Deal with refueling
   p = ai.getPlayer()
   if ai.exists(p) then
      standing = ai.getstanding( p ) or -1
      mem.refuel = rnd.rnd( 2000, 4000 )
      if standing > 60 then
         mem.refuel = mem.refuel * 0.5
      end
      mem.refuel_msg = string.format("\"For you, only %d credits for fuel for a jump.\"",
            mem.refuel);
   end


   -- Choose attack format
   attack_choose()
end


function taunt ( target, offense )

   -- Only 50% of actually taunting.
   if rnd.rnd(0,1) == 0 then
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

   ai.comm(target, taunts[ rnd.rnd(1,#taunts) ])
end

