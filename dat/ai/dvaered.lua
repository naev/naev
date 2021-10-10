require 'ai.core.core'
local fmt = require "format"

-- Settings
mem.aggressive = true
mem.whiteknight = true
mem.formation = "wall"

local bribe_no_list = {
   _([["You insult my honour."]]),
   _([["I find your lack of honour disturbing."]]),
   _([["You disgust me."]]),
   _([["Bribery carries a harsh penalty."]]),
   _([["House Dvaered does not lower itself to common scum."]])
}
local taunt_list = {
   _("Prepare to face annihilation!"),
   _("I shall wash my hull in your blood!"),
   _("Your head will make a great trophy!"),
   _("You're no match for the Dvaered!"),
   _("Death awaits you!")
}

-- Create function
function create ()
   local p = ai.pilot()
   local ps = p:ship()

   -- Credits.
   ai.setcredits( rnd.rnd(ps:price()/300, ps:price()/100) )

   -- Set how far they attack
   mem.enemyclose = 3000 * ps:size()

   create_post()
end

-- When hailed
function hail ()
   local p = ai.pilot()

   -- Remove randomness from future calls
   if not mem.hailsetup then
      mem.refuel_base = rnd.rnd( 1000, 3000 )
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

   -- Handle refueling
   local standing = p:faction():playerStanding()
   if standing < 50 then
      mem.refuel_no = _([["You are not worthy of my attention."]])
   else
      mem.refuel = mem.refuel_base
      mem.refuel_msg = string.format(_([["For you I could make an exception for %s."]]), fmt.credits(mem.refuel))
   end

   -- Handle bribing
   if rnd.rnd() > 0.4 then
      mem.bribe_no = _([["I shall especially enjoy your death."]])
   else
      mem.bribe_no = bribe_no_list[ rnd.rnd(1,#bribe_no_list) ]
   end
end

-- taunts
function taunt ( target, _offense )
   -- Only 50% of actually taunting.
   if rnd.rnd(0,1) == 0 then
      return
   end

   -- Offense is not actually used
   local taunts = taunt_list
   ai.pilot():comm( target, taunts[ rnd.rnd(1,#taunts) ] )
end

