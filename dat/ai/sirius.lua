require 'ai.core.core'
local fmt = require "format"

-- Settings
mem.armour_run    = 0
mem.armour_return = 0
mem.aggressive    = true
mem.whiteknight   = true
mem.formation     = "wedge"

local bribe_no_list = {
   _([["Your money is of no interest to me."]]),
   _([["I have no need of your dirty credits."]]),
}
local taunt_list_offensive = {
   _("The universe shall be cleansed of your presence!"),
   _("Time for some cleansing!"),
   _("House Sirius has no need of you. Begone!"),
   _("Prepare to pay for your sins!"),
}
local taunt_list_defensive = {
   _("Sirichana protect me!"),
   _("You have made a grave error!"),
   _("You do wrong in your provocations!"),
   _("I shall overcome this test!"),
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
      ai.setcredits( rnd.rnd(price/200, price/50) )
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
      mem.refuel_base = rnd.rnd( 1000, 3000 )
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

   -- Deal with refueling
   local standing = p:faction():playerStanding()
   mem.refuel = mem.refuel_base
   if standing < 0 then
      mem.refuel_no = _([["I do not have fuel to spare."]])
   elseif standing > 30 then
      if mem.refuel_rng < 0.5 then
         mem.refuel_no = _([["I do not have fuel to spare."]])
      end
   else
      mem.refuel = mem.refuel * 0.6
   end
   -- Most likely no chance to refuel
   mem.refuel_msg = fmt.f( _([["I would be able to refuel your ship for {credits}."]]), {credits=fmt.credits(mem.refuel)} )

   -- Can't be bribed
   mem.bribe_no = bribe_no_list[ rnd.rnd(1,#bribe_no_list) ]
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
