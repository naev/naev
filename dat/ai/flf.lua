require 'ai.core.core'
require 'ai.core.idle.pirate'
require "numstring"

-- Settings
mem.aggressive    = true
mem.safe_distance = 300
mem.armour_run    = 100
mem.shield_return = 20
mem.land_planet   = false
mem.careful       = true
mem.doscans       = false

local taunt_list_offensive = {
   _("For the Frontier!"),
   _("You'll make great target practice!"),
   _("Purge the oppressors!"),
}
local taunt_list_defensive= {
   _("You are no match for the FLF."),
   _("I've killed scum far more dangerous than you."),
   _("You'll regret that!"),
   _("Death to the enemies of the Frontier!"),
}

function create ()
   local p = ai.pilot()
   local ps = ai.pilot():ship()

   -- Give monies.
   ai.setcredits( rnd.rnd(ps:price()/600, ps:price()/100) )

   -- Get standing.
   local pp = player.pilot()
   if pp:exists() then
      local standing = ai.getstanding( pp ) or -1

      -- Handle bribes.
      mem.bribe = math.sqrt( p:stats().mass ) * (300. * rnd.rnd() + 850.)
      if standing > -30 or
            (standing > -60 and rnd.rnd() > 0.8) or
            (rnd.rnd() > 0.4) then
         mem.bribe_prompt = string.format(_([["It'll cost you %s for me to ignore your dirty presence."]]), creditstring(mem.bribe))
         mem.bribe_paid = _([["Begone before I change my mind."]])
      else
         mem.bribe_no = _([["The only way to deal with scum like you is with cannons!"]])
      end

      -- Handle refueling.
      if standing > 70 or
            (standing > 30 and rnd.rnd() > 0.8) or
            (standing > 0 and rnd.rnd() > 0.5) then
         mem.refuel = rnd.rnd( 1000, 2000 )
         mem.refuel_msg = string.format(_([["I should be able to spare some fuel for %s."]]), creditstring(mem.refuel))
      else
         mem.refuel_no = _([["I can't spare fuel for you."]])
      end
   end

   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system


   -- Set how far they attack
   mem.ambushclose = 4000 + 1000 * ps:size()
   mem.stealth = p:flags("stealth")

   -- Finish up creation
   create_post()
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

