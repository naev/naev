require 'ai.core.core'
local fmt = require "format"

-- Settings
mem.armour_run    = 40
mem.armour_return = 70
mem.aggressive    = true
mem.whiteknight   = true
mem.formation     = "vee"

local bribe_no_list = {
   _([["I shall especially enjoy your death."]]),
   _([["Snivelling waste of carbon."]]),
   _([["Money won't save you from being purged from the gene pool."]]),
   _([["Culling you will be doing humanity a service."]]),
   _([["We do not consort with vermin."]]),
   _([["Who do you take us for, the Empire?"]])
}
local taunt_list_offensive = {
   _("There is no room in this universe for scum like you!"),
   _("Culling you will be doing humanity a service."),
   _("Enjoy your last moments, worm!"),
   _("Time for a little natural selection!"),
   _("Might makes right!"),
   _("Embrace your weakness!")
}
local taunt_list_defensive= {
   _("Cunning, but foolish."),
   _("Ambush! Defend yourselves!"),
   _("You should have picked easier prey!"),
   _("You'll regret that!"),
   _("That was a fatal mistake!")
}

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

   -- Set how far they attack
   mem.enemyclose = 2000 + 2000 * ps:size()

   -- Finish up creation
   create_post()
end

function hail ()
   local p = ai.pilot()

   -- Remove randomness from future calls
   if not mem.hailsetup then
      mem.refuel_base = mem.refuel_base or rnd.rnd( 2000, 4000 )
      mem.refuel_rng = rnd.rnd()
      mem.bribe_base = mem.bribe_base or 3*math.sqrt( p:mass() ) * (500 * rnd.rnd() + 1750)
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
   if standing < 0 then
      mem.refuel_no = _([["The warriors of Sorom are not your personal refueller."]])
   elseif standing < 40 then
      if mem.refuel_rng > 0.4 then
         mem.refuel_no = _([["The warriors of Sorom are not your personal refueller."]])
      end
   else
      mem.refuel = mem.refuel * 0.5
   end
   -- Most likely no chance to refuel
   mem.refuel_msg = fmt.f( _([["I suppose I could spare some fuel for {credits}."]]), {credits=fmt.credits(mem.refuel)} )

   -- Handle bribing
   mem.bribe = mem.bribe_base
   if mem.found_illegal or mem.allowbribe or (mem.natural and (standing > 20 or
         (standing > 0 and mem.bribe_rng > 0.8) or
         (standing > -20 and mem.bribe_rng > 0.6) or
         (standing > -50 and mem.bribe_rng > 0.4) or
         (rnd.rnd() > 0.2))) then
      mem.bribe_prompt = fmt.f(_([["For {credits} I'll give you enough time to get out of my sight."]]), {credits=fmt.credits(mem.bribe)} )
      mem.bribe_paid = _([["Now get out of my sight."]])
   else
      mem.bribe_no = bribe_no_list[ rnd.rnd(1,#bribe_no_list) ]
   end
end

-- taunts
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
