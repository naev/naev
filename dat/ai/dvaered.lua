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
local taunt_list_warship = {
   _("I shall wash my hull in your blood!"),
   _("Your head will make a great trophy!"),
}
local taunt_list_default = {
   _("Prepare to face annihilation!"),
   _("You're no match for the Dvaered!"),
   _("Death awaits you!")
}
tmergei( taunt_list_warship, taunt_list_default ) -- Add default to warship

-- Create function
function create ()
   create_pre()
   local p = ai.pilot()
   local ps = p:ship()
   local price = ps:price()

   -- See if it's a transport ship
   mem.istransport = ps:tags().transport

   -- Credits, and other transport-specific stuff
   if mem.istransport then
      transportParam( price )
   else
      ai.setcredits( rnd.rnd(price/300, price/100) )
   end

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
      mem.refuel_msg = fmt.f(_([["For you I could make an exception for {credits}."]]), {credits=fmt.credits(mem.refuel)})
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
   local taunts
   if mem.istransport then
      taunts = taunt_list_default
   else
      taunts = taunt_list_warship
   end

   ai.pilot():comm( target, taunts[ rnd.rnd(1,#taunts) ] )
end
