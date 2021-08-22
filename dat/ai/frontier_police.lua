require 'ai.core.core'
require "numstring"

-- Settings
mem.aggressive = true

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
   -- Credits.
   ai.setcredits( rnd.rnd(ai.pilot():ship():price()/300, ai.pilot():ship():price()/100) )

   -- Handle misc stuff
   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system

   create_post()
end

-- When hailed
function hail ()
   if mem.setuphail then return end

   -- Handle refueling
   mem.refuel = rnd.rnd( 1000, 3000 )
   local standing = ai.getstanding( player.pilot() ) or -1
   if standing > 50 or
         (standing > 0 and rnd.rnd() > 0.8) or
         (rnd.rnd() > 0.3) then
      mem.refuel_no = _([["Mare magno turbantibus. That means that I don't care about your problems."]])
   else
      mem.refuel_msg = string.format(_([["For you I could make an exception for %s."]]), creditstring(mem.refuel))
   end

   -- Handle bribing
   mem.bribe = math.sqrt( ai.pilot():stats().mass ) * (750 * rnd.rnd() + 2500)
   if (mem.natural or mem.allowbribe) and (standing > 0 or
         (standing > -20 and rnd.rnd() > 0.8) or
         (standing > -50 and rnd.rnd() > 0.5) or
         (rnd.rnd() > 0.3)) then
      mem.bribe_prompt = string.format(_([["For %s I'll let your grievances slide."]]), creditstring(mem.bribe) )
      mem.bribe_paid = _([["Now get out of my sight and don't cause any more trouble."]])
   else
      mem.bribe_no = bribe_no[ rnd.rnd(1,#bribe_no) ]
   end

   mem.setuphail = true
end

-- taunts
function taunt ( target, offense )
   -- Only 50% of actually taunting.
   if rnd.rnd(0,1) == 0 then
      return
   end

   -- Offense is not actually used
   local taunts = taunt_list
   ai.pilot():comm( target, taunts[ rnd.rnd(1,#taunts) ] )
end

