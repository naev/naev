require 'ai.core.core'
require "numstring"

-- Settings
mem.armour_run    = 0
mem.armour_return = 0
mem.aggressive    = true

local bribe_no_list = {
   _([["Your money is of no interest to me."]])
}
local taunt_list_offensive = {
   _("The universe shall be cleansed of your presence!")
}
local taunt_list_defensive = {
   _("Sirichana protect me!"),
   _("You have made a grave error!"),
   _("You do wrong in your provocations!")
}

function create ()
   local p = ai.pilot()
   local ps = p:ship()

   -- Not too many credits.
   ai.setcredits( rnd.rnd(ps:price()/200, ps:price()/50) )

   mem.loiter = 2 -- This is the amount of waypoints the pilot will pass through before leaving the system

   -- Set how far they attack
   mem.enemyclose = 2000 + 2000 * ps:size()

   -- Finish up creation
   create_post()
end

function hail ()
   if mem.setuphail then return end

   -- Get refuel chance
   local standing = ai.getstanding( player.pilot() ) or -1
   mem.refuel = rnd.rnd( 1000, 2000 )
   if standing < 0 then
      mem.refuel_no = _([["I do not have fuel to spare."]])
   elseif standing > 30 then
      if rnd.rnd() < 0.5 then
         mem.refuel_no = _([["I do not have fuel to spare."]])
      end
   else
      mem.refuel = mem.refuel * 0.6
   end
   -- Most likely no chance to refuel
   mem.refuel_msg = string.format( _([["I would be able to refuel your ship for %s."]]), creditstring(mem.refuel) )

   -- Can't be bribed
   mem.bribe_no = bribe_no_list[ rnd.rnd(1,#bribe_no_list) ]

   mem.setuphail = true
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


