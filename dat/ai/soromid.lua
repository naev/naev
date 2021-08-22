require 'ai.core.core'
require "numstring"

-- Settings
mem.armour_run    = 40
mem.armour_return = 70
mem.aggressive    = true

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
   local p = ai.pilot()
   local ps = p:ship()

   -- Not too many credits.
   ai.setcredits( rnd.rnd(ps:price()/300, ps:price()/70) )

   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system

   -- Set how far they attack
   mem.enemyclose = 2000 + 2000 * ps:size()

   -- Finish up creation
   create_post()
end

function hail ()
   -- Get refuel chance
   if mem.refuel == nil then
      local standing = ai.getstanding( player.pilot() ) or -1
      mem.refuel = rnd.rnd( 2000, 4000 )
      if standing < 0 then
         mem.refuel_no = _([["The warriors of Sorom are not your personal refueller."]])
      elseif standing < 40 then
         if rnd.rnd() > 0.4 then
            mem.refuel_no = _([["The warriors of Sorom are not your personal refueller."]])
         end
      else
         mem.refuel = mem.refuel * 0.5
      end
      -- Most likely no chance to refuel
      mem.refuel_msg = string.format( _([["I suppose I could spare some fuel for %s."]]), creditstring(mem.refuel) )

      -- Handle bribing
      mem.bribe = 3 * math.sqrt( ai.pilot():stats().mass ) * (500 * rnd.rnd() + 1750)
      if (mem.natural or mem.allowbribe) and (standing > 20 or
            (standing > 0 and rnd.rnd() > 0.8) or
            (standing > -20 and rnd.rnd() > 0.6) or
            (standing > -50 and rnd.rnd() > 0.4) or
            (rnd.rnd() > 0.2)) then
         mem.bribe_prompt = string.format(_([["For %s I'll give you enough time to get out of my sight."]]), creditstring(mem.bribe) )
         mem.bribe_paid = _([["Now get out of my sight."]])
      else
         mem.bribe_no = bribe_no_list[ rnd.rnd(1,#bribe_no_list) ]
      end
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


