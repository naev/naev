require 'ai.core.core'
require 'ai.core.idle.pirate'
local fmt = require "format"

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
   create_pre()

   local p = ai.pilot()
   local ps = ai.pilot():ship()

   -- Give monies.
   ai.setcredits( rnd.rnd(ps:price()/400, ps:price()/100) )

   -- Set how far they attack
   mem.ambushclose = 4000 + 1000 * ps:size()
   mem.enemyclose = mem.ambushclose
   mem.stealth = p:flags("stealth")

   -- Finish up creation
   create_post()
end


function hail ()
   local p = ai.pilot()

   -- Remove randomness from future calls
   if not mem.hailsetup then
      mem.refuel_base = mem.refuel_base or rnd.rnd( 1000, 2000 )
      mem.bribe_base = mem.bribe_base or math.sqrt( p:mass() ) * (300 * rnd.rnd() + 850)
      mem.bribe_rng = rnd.rnd()
      mem.refuel_rng = rnd.rnd()
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

   local standing = p:faction():playerStanding()

   -- Handle bribes.
   mem.bribe = mem.bribe_base
   if mem.allowbribe or (mem.natural and (standing > -30 or
         (standing > -60 and mem.bribe_rng > 0.8) or
         (mem.bribe_rng > 0.4))) then
      mem.bribe_prompt = fmt.f(_([["It'll cost you {credits} for me to ignore your dirty presence."]]), {credits=fmt.credits(mem.bribe)})
      mem.bribe_paid = _([["Begone before I change my mind."]])
   else
      mem.bribe_no = _([["The only way to deal with scum like you is with cannons!"]])
   end

   -- Handle refueling.
   if standing > 70 or
         (standing > 30 and mem.refuel_rng > 0.8) or
         (standing > 0 and mem.refuel_rng > 0.5) then
      mem.refuel = mem.refuel_base
      mem.refuel_msg = fmt.f(_([["I should be able to spare some fuel for {credits}."]]), {credits=fmt.credits(mem.refuel)})
   else
      mem.refuel_no = _([["I can't spare fuel for you."]])
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
