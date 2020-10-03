require("ai/tpl/generic.lua")
require("ai/personality/patrol.lua")

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
mem.careful       = true


function create ()

   -- Some pirates do kill
   if rnd.rnd() < 0.1 then
      mem.atk_kill = true
   end

   -- Not too much money
   ai.setcredits( rnd.int(ai.pilot():ship():price()/80 , ai.pilot():ship():price()/30) )

   -- Deal with bribeability
   if rnd.rnd() < 0.05 then
      mem.bribe_no = _("\"You won't be able to slide out of this one!\"")
   else
      mem.bribe = math.sqrt( ai.pilot():stats().mass ) * (300. * rnd.rnd() + 850.)
      bribe_prompt = {
            _("\"It'll cost you %d credits for me to ignore your pile of rubbish.\""),
            _("\"I'm in a good mood so I'll let you go for %d credits.\""),
            _("\"Send me %d credits or you're dead.\""),
            _("\"Pay up %d credits or it's the end of the line.\""),
            _("\"Your money or your life. %d credits and make the choice quickly.\""),
            _("\"Money talks bub. %d up front or get off my channel.\""),
            _("\"Shut up and give me your money! %d credits now.\""),
            _("\"You're either really desperate or really rich. %d or shut up.\""),
            _("\"If you're willing to negotiate I'll gladly take %d credits to not kill you.\""),
            _("\"You give me %d credits and I'll act like I never saw you.\""),
            _("\"So this is the part where you pay up or get shot up. Your choice. What'll be, %d or...\""),
            _("\"Pay up or don't. %d credits now just means I'll wait till later to collect the rest.\""),
            _("\"This is a toll road, pay up %d credits or die.\""),
      }
      mem.bribe_prompt = string.format(bribe_prompt[ rnd.rnd(1,#bribe_prompt) ], mem.bribe)
      bribe_paid = {
            _("\"You're lucky I'm so kind.\""),
            _("\"Life doesn't get easier than this.\""),
            _("\"Pleasure doing business.\""),
            _("\"See you again, real soon.\""),
            _("\"I'll be around if you get generous again.\""),
            _("\"Lucky day, lucky day!\""),
            _("\"And I didn't even have to kill anyone!\""),
            _("\"See, this is how we become friends.\""),
            _("\"Now if I kill you it'll be just for fun!\""),
            _("\"You just made a good financial decision today.\""),
            _("\"Know what? I won't kill you.\""),
            _("\"Something feels strange. It's almost as if my urge to kill you has completely dissipated.\""),
            _("\"Can I keep shooting you anyhow? No? You sure? Fine.\""),
            _("\"And it only cost you an arm and a leg.\""),
      }
      mem.bribe_paid = bribe_paid[ rnd.rnd(1,#bribe_paid) ]
   end

   -- Deal with refueling
   p = player.pilot()
   if p:exists() then
      standing = ai.getstanding( p ) or -1
      mem.refuel = rnd.rnd( 2000, 4000 )
      if standing > 60 then
         mem.refuel = mem.refuel * 0.5
      end
      mem.refuel_msg = string.format(_("\"For you, only %d credits for a hundred units of fuel.\""),
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
            _("Prepare to be boarded!"),
            _("Yohoho!"),
            _("Arr!"),
            _("What's a ship like you doing in a place like this?"),
            _("Prepare to have your booty plundered!"),
            _("Give me your credits or die!"),
            _("Your ship's mine!"),
            _("Oh ho ho, what do I see here?"),
            _("You may want to send that distress signal now."),
            _("It's time to die."),
            _("Back so soon?"),
            _("What? Were you expecting prince charming?"),
            _("Long time no see."),
            _("Nothing personal, just business."),
            _("Nothing personal."),
            _("Just business."),
            _("I can already taste the rum."),
            _("How else am I going to pay off my tab?"),
            _("Seems you're being shot at."),
            _("I'm trying to kill you. Is it working?"),
            _("I'm sorry, I just don't like you."),
            _("Sorry, but I'm a private tracker."),
      }
   else
      taunts = {
            _("You dare attack me?!"),
            _("You think that you can take me on?"),
            _("Die!"),
            _("You'll regret this!"),
            _("You can either pray now or sit in hell later."),
            _("Game over, you're dead!"),
            _("I'm sorry things couldn't work out between us."),
            _("Knock it off!"),
            _("Shooting back isn't allowed!"),
            _("You owe me 20 credits!"),
            _("You owe me a new paint job!"),
            _("Fred here said to shoot the red blip."),
            _("Now you're in for it!"),
            _("Did you really think you would get away with that?"),
            _("I just painted this thing!"),
            _("Rot."),
            _("Burn."),
            _("I can't wait to see you burn."),
            _("Just. Stop. Moving."),
            _("Die already!"),
            _("Tell you what, if you can keep dodging for 20 hectoseconds I'll let you live."),
            _("Stop dodging!"),
            _("Okay, that's enough of that!"),
            _("I'm gonna torrent you to bits!"),
      }
   end

   ai.pilot():comm(target, taunts[ rnd.rnd(1,#taunts) ])
end

