include("dat/ai/tpl/generic.lua")
include("dat/ai/personality/patrol.lua")

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
   if rnd.rnd() < 0.1 then
      mem.atk_kill = true
   end

   -- Not too much money
   ai.setcredits( rnd.int(ai.shipprice()/80 , ai.shipprice()/30) )

   -- Deal with bribeability
   if rnd.rnd() < 0.05 then
      mem.bribe_no = "\"You won't be able to slide out of this one!\""
   else
      mem.bribe = math.sqrt( ai.shipmass() ) * (300. * rnd.rnd() + 850.)
      bribe_prompt = {
            "\"It'll cost you %d credits for me to ignore your pile of rubbish.\"",
            "\"I'm in a good mood so I'll let you go for %d credits.\"",
            "\"Send me %d credits or you're dead.\"",
            "\"Pay up %d credits or it's the end of the line.\""
      }
      mem.bribe_prompt = string.format(bribe_prompt[ rnd.rnd(1,#bribe_prompt) ], mem.bribe)
      bribe_paid = {
           "\"You're lucky I'm so kind.\"",
            "\"Life doesn't get easier than this.\"",
            "\"Pleasure doing business.\"",
            "\"See you again, real soon.\""
      }
      mem.bribe_paid = bribe_paid[ rnd.rnd(1,#bribe_paid) ]
   end

   -- Deal with refueling
   p = ai.getPlayer()
   if ai.exists(p) then
      standing = ai.getstanding( p ) or -1
      mem.refuel = rnd.rnd( 2000, 4000 )
      if standing > 60 then
         mem.refuel = mem.refuel * 0.5
      end
      mem.refuel_msg = string.format("\"For you, only %d credits for a hundred units of fuel.\"",
            mem.refuel);
   end

   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system

   -- Finish up creation
   create_post()
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
            "Arr!",
            "What's a ship like you doing in a place like this?",
            "Prepare to have your booty plundered!",
            "Give me your credits or die!",
            "Your ship's mine!"
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

