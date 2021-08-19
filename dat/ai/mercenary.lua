require 'ai.core.core'
require "numstring"

-- Settings
mem.armour_run    = 40
mem.armour_return = 70
mem.aggressive    = true

local bribe_no_list = {
   _([["You won't buy your way out of this one."]]),
   _([["I'm afraid you can't make it worth my while."]]),
}
local taunt_list_offensive = {
   _("Don't take this personally."),
   _("It's just business."),
}
local taunt_list_defensive = {
   _("Your skull will make a great hood ornament."),
   _("I've destroyed ships twice the size of yours!"),
   _("I'll crush you like a grape!"),
   _("This isn't what I signed up for!"),
}

function create ()
   ai.setcredits( rnd.rnd(ai.pilot():ship():price()/150, ai.pilot():ship():price()/50) )

   mem.bribe = math.sqrt( ai.pilot():stats().mass ) * (750 * rnd.rnd() + 2500)
   if rnd.rnd() > 0.7 then
      mem.bribe_prompt = string.format(_([["Your life is worth %s to me."]]), creditstring(mem.bribe) )
      mem.bribe_paid = _([["Beat it."]])
   else
      mem.bribe_no = bribe_no_list[ rnd.rnd(1,#bribe_no_list) ]
   end

   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system

   -- Finish up creation
   create_post()
end

-- When hailed
function hail ()
   -- Refuel
   local pp = player.pilot()
   if pp:exists() and mem.refuel == nil then
      mem.refuel = rnd.rnd( 3000, 5000 )
      mem.refuel_msg = string.format(_([["I'll supply your ship with fuel for %s."]]),
            creditstring(mem.refuel))
   end
end

-- taunts
function taunt ( target, offense )
   -- Only 20% of actually taunting.
   if rnd.rnd() > 0.2 then
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


