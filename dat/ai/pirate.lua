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
            "\"Pay up %d credits or it's the end of the line.\"",
            "\"Your money or your life. %d credits and make the choice quickly.\"",
            "\"Money talks bub. %d up front or get off my channel.\"",
            "\"Shut up and give me your money! %d credits now.\"",
            "\"You're either really stupid to be chatting or really rich. %d or shut up.\"",
            "\"If you're willing to negotiate I'll gladly take %d credits to not kill you.\"",
            "\"You give me %d credits and I'll act like I never saw you.\"",
            "\"So this is the part where you pay up or get shot up. Your choice. What'll be, %d or...\"",
            "\"Pay up or don't. %d credits now just means I'll wait till later to collect the rest.\"",
            "\"This is a toll road, pay up %d credits or die.\"",
      }
      mem.bribe_prompt = string.format(bribe_prompt[ rnd.rnd(1,#bribe_prompt) ], mem.bribe)
      bribe_paid = {
            "\"You're lucky I'm so kind.\"",
            "\"Life doesn't get easier than this.\"",
            "\"Pleasure doing business.\"",
            "\"See you again, real soon.\"",
            "\"I'll be around if you get generous again.\"",
            "\"Lucky day, lucky day!\"",
            "\"And I didn't even have to kill anyone!\"",
            "\"See, this is how we become friends.\"",
            "\"Now if I kill you it'll be just for fun!\"",
            "\"You just made a good financial decision today.\"",
            "\"Know what? I won't kill you.\"",
            "\"Something feels strange. It's almost as if my urge to kill you has completely dissipated.\"",
            "\"Can I keep shooting you anyhow? No? You sure? Fine.\"",
            "\"And it only cost you an arm and a leg.\"",
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
            "Your ship's mine!",
            "Oh ho ho, what do I see here?",
            "You may want to send that distress signal now.",
            "It's time to die.",
            "Back so soon?",
            "What? Were you expecting prince charming?",
            "Long time no see.",
            "Nothing personal, just business.",
            "Nothing personal.",
            "Just business.",
            "I can already taste the rum.",
            "How else am I going to pay off my tab?",
            "Seems you're being shot at.",
            "I'm trying to kill you. Is it working?",
            "I'm sorry, I just don't like you.",
      }
   else
      taunts = {
            "You dare attack me?!",
            "You think that you can take me on?",
            "Die!",
            "You'll regret this!",
            "You can either pray now or sit in hell later.",
            "Game over, you're dead!",
            "I'm sorry things couldn't work out between us.",
            "Knock it off!",
            "Shooting back isn't allowed!",
            "You owe me 20 credits!",
            "You owe me a new paint job!",
            "Fred here said to shoot the red blip.",
            "Now you're in for it!",
            "Did you really think you would get away with that?",
            "I just painted this thing!",
            "Rot.",
            "Burn.",
            "I can't wait to see you burn.",
            "Just. Stop. Moving.",
            "Die already!",
            "Tell you what, if you can keep dodging for 20 minutes I'll let you live.",
            "Stop dodging!",
            "Okay, that's enough of that!",
      }
   end

   ai.comm(target, taunts[ rnd.rnd(1,#taunts) ])
end

