require 'ai.core.core'
require 'ai.core.idle.pirate'
local fmt = require "format"

--[[
   The Glorious Pirate AI
--]]

-- Settings
mem.aggressive    = true
mem.safe_distance = 500
mem.armour_run    = 80
mem.armour_return = 100
mem.atk_board     = true
mem.atk_kill      = false
mem.careful       = true

local function join_tables( a, b )
   for _k,v in ipairs(b) do
      table.insert( a, v )
   end
   return a
end

local bribe_prompt_common = {
   _([["Pay up {credits} or it's the end of the line."]]),
   _([["Your money or your life. {credits} and make the choice quickly."]]),
   _([["Money talks bub. {credits} up front or get off my channel."]]),
   _([["You're either really desperate or really rich. {credits} or shut up."]]),
   _([["This is a toll road, pay up {credits} or die."]]),
   _([["So this is the part where you pay up or get shot up. Your choice. What'll be, {credits} or…"]]),
}
local bribe_prompt_list = join_tables( {
   _([["It'll cost you {credits} for me to ignore your pile of rubbish."]]),
   _([["I'm in a good mood so I'll let you go for {credits}."]]),
   _([["Send me {credits} or you're dead."]]),
   _([["Shut up and give me your money! {credits} now."]]),
   _([["You give me {credits} and I'll act like I never saw you."]]),
   _([["If you're willing to negotiate I'll gladly take {credits} to not kill you."]]),
   _([["Pay up or don't. {credits} now just means I'll wait till later to collect the rest."]]),
}, bribe_prompt_common )
local bribe_prompt_nearby_list = join_tables( {
   _([["It'll cost you {credits} for us to ignore your pile of rubbish."]]),
   _([["We're in a good mood so we'll let you go for {credits}."]]),
   _([["Send us {credits} or you're dead."]]),
   _([["Shut up and give us your money! {credits} now."]]),
   _([["You give us {credits} and we'll act like we never saw you."]]),
   _([["If you're willing to negotiate we'll gladly take {credits} to not kill you."]]),
   _([["Pay up or don't. {credits} now just means we'll wait till later to collect the rest."]]),
}, bribe_prompt_common )
local bribe_paid_list = {
   _([["You're lucky I'm so kind."]]),
   _([["Life doesn't get easier than this."]]),
   _([["Pleasure doing business."]]),
   _([["See you again, real soon."]]),
   _([["I'll be around if you get generous again."]]),
   _([["Lucky day, lucky day!"]]),
   _([["And I didn't even have to kill anyone!"]]),
   _([["See, this is how we become friends."]]),
   _([["Now if I kill you it'll be just for fun!"]]),
   _([["You just made a good financial decision today."]]),
   _([["Know what? I won't kill you."]]),
   _([["Something feels strange. It's almost as if my urge to kill you has completely dissipated."]]),
   _([["Can I keep shooting you anyhow? No? You sure? Fine."]]),
   _([["And it only cost you an arm and a leg."]]),
}
local taunt_list_offensive = {
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
   _("Hey ho! Space pirates! Rum-dee Rum Rum!"), -- From the space pirate shanty
}
local taunt_list_defensive = {
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
local pir_formations = {
   "echelon_left",
   "echelon_right",
   "vee",
   "wedge",
   "cross",
}

function create ()
   create_pre()

   local p = ai.pilot()
   local ps = ai.pilot():ship()

   -- Some pirates do kill
   if rnd.rnd() < 0.1 then
      mem.atk_kill = true
   end

   -- Not too much money
   ai.setcredits( rnd.rnd(ps:price()/80, ps:price()/30) )

   -- Set how far they attack
   mem.ambushclose= 4000 + 1000 * ps:size()
   mem.enemyclose = mem.ambushclose
   mem.stealth    = p:flags("stealth") -- Follow however they were spawned
   mem.formation  = pir_formations[ rnd.rnd(1,#pir_formations) ]

   -- Finish up creation
   create_post()
end


function hail ()
   local p = ai.pilot()

   -- Remove randomness from future calls
   if not mem.hailsetup then
      mem.refuel_base = mem.refuel_base or rnd.rnd( 2000, 4000 )
      if not mem.bribe_base then
         local pp = player.pilot()
         local worth = pp:worth()
         for k,v in ipairs(pp:followers()) do
            if not p:flags("carried") then
               worth = worth+v:worth()
            end
         end
         mem.bribe_base = (150 * rnd.rnd() + 425) * p:ship():points() * math.max( 0.5, worth / 700e3 )
      end
      mem.bribe_rng = rnd.rnd()
      mem.hailsetup = true
   end

   -- Clean up
   mem.refuel        = 0
   mem.refuel_msg    = nil
   mem.bribe         = 0
   mem.bribe_prompt  = nil
   mem.bribe_prompt_nearby = nil
   mem.bribe_paid    = nil
   mem.bribe_no      = nil

   -- Deal with refueling
   local standing = p:faction():playerStanding()
   mem.refuel = mem.refuel_base
   if standing > 60 then
      mem.refuel = mem.refuel * 0.5
   end
   mem.refuel_msg = fmt.f(_([["For you, only {credits} for a jump of fuel."]]), {credits=fmt.credits(mem.refuel)})

   -- Deal with bribeability
   mem.bribe = mem.bribe_base
   if mem.allowbribe or (mem.natural and mem.bribe_rng < 0.95) then
      mem.bribe_prompt = fmt.f(bribe_prompt_list[ rnd.rnd(1,#bribe_prompt_list) ], {credits=fmt.credits(mem.bribe)})
      mem.bribe_prompt_nearby = bribe_prompt_nearby_list[ rnd.rnd(1,#bribe_prompt_nearby_list) ]
      mem.bribe_paid = bribe_paid_list[ rnd.rnd(1,#bribe_paid_list) ]
   else
      mem.bribe_no = _([["You won't be able to slide out of this one!"]])
   end
end


function taunt ( target, offense )
   -- Only 50% of actually taunting.
   if rnd.rnd(0,1) == 0 then
      return
   end

   -- some taunts
   local taunts
   if offense then
      taunts = taunt_list_offensive
   else
      taunts = taunt_list_defensive
   end

   ai.pilot():comm(target, taunts[ rnd.rnd(1,#taunts) ])
end
