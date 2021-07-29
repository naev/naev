require 'ai.core.core'
require "numstring"

-- Settings
mem.aggressive = true

-- Create function
function create ()
   local p = ai.pilot()
   local ps = p:ship()

   -- Credits.
   ai.setcredits( rnd.rnd(ps:price()/300, ps:price()/100) )

   -- Handle bribing
   if rnd.rnd() > 0.4 then
      mem.bribe_no = _("\"I shall especially enjoy your death.\"")
   else
      local bribe_no = {
            _("\"You insult my honour.\""),
            _("\"I find your lack of honour disturbing.\""),
            _("\"You disgust me.\""),
            _("\"Bribery carries a harsh penalty.\""),
            _("\"House Dvaered does not lower itself to common scum.\"")
     }
     mem.bribe_no = bribe_no[ rnd.rnd(1,#bribe_no) ]
   end

   -- Handle refueling
   local pp = player.pilot()
   if pp:exists() then
      local standing = ai.getstanding( pp ) or -1
      mem.refuel = rnd.rnd( 1000, 3000 )
      if standing < 50 then
         mem.refuel_no = _("\"You are not worthy of my attention.\"")
      else
         mem.refuel_msg = string.format(_("\"For you I could make an exception for %s.\""), creditstring(mem.refuel))
      end
   end

   -- Handle misc stuff
   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system

   -- Set how far they attack
   mem.enemyclose = 3000 * ps:size()

   create_post()
end

-- taunts
function taunt ( target, offense )
   -- Only 50% of actually taunting.
   if rnd.rnd(0,1) == 0 then
      return
   end

   -- Offense is not actually used
   local taunts = {
       _("Prepare to face annihilation!"),
       _("I shall wash my hull in your blood!"),
       _("Your head will make a great trophy!"),
       _("You're no match for the Dvaered!"),
       _("Death awaits you!")
   }
   ai.pilot():comm( target, taunts[ rnd.rnd(1,#taunts) ] )
end

