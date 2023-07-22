require 'ai.core.core'
local fmt = require "format"

-- Settings
mem.aggressive    = true
mem.whiteknight   = true
mem.formation     = "wedge"

local bribe_no_list = {
   _([["I shall especially enjoy your death."]]),
   _([["You insult my honour."]]),
   _([["I find your lack of honour disturbing."]]),
   _([["You disgust me."]]),
   _([["Bribery carries a harsh penalty."]]),
   _([["The Frontier does not lower itself to common scum."]]),
}
local taunt_list = {
   _("Alea iacta est!"),
   _("Morituri te salutant!"),
   _("Your head will make a great trophy!"),
   _("Cave canem!"),
   _("Death awaits you!"),
}

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
      ai.setcredits( rnd.rnd(price/300, price/70) )
   end

   create_post()
end

-- When hailed
function hail ()
   local p = ai.pilot()

   -- Remove randomness from future calls
   if not mem.hailsetup then
      mem.refuel_base = mem.refuel_base or rnd.rnd( 1000, 3000 )
      mem.refuel_rng = rnd.rnd()
      mem.bribe_base = mem.bribe_base or math.sqrt( p:mass() ) * (750 * rnd.rnd() + 2500)
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

   -- Get refuel chance
   local standing = p:faction():playerStanding()
   mem.refuel = mem.refuel_base
   if standing > 50 or
         (standing > 0 and mem.refuel_rng > 0.8) or
         (mem.refuel_rng > 0.3) then
      mem.refuel_no = _([["Mare magno turbantibus. That means that I don't care about your problems."]])
   else
      mem.refuel_msg = fmt.f(_([["For you I could make an exception for {credits}."]]), {credits=fmt.credits(mem.refuel)})
   end

   -- Handle bribing
   mem.bribe = mem.bribe_base
   if mem.allowbribe or (mem.natural and (standing > 0 or
         (standing > -20 and mem.bribe_rng > 0.8) or
         (standing > -50 and mem.bribe_rng > 0.5) or
         (mem.bribe_rng > 0.3))) then
      mem.bribe_prompt = fmt.f(_([["For {credits} I'll let your grievances slide."]]), {credits=fmt.credits(mem.bribe)} )
      mem.bribe_paid = _([["Now get out of my sight and don't cause any more trouble."]])
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
