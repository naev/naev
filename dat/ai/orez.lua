require 'ai.core.core'
local fmt = require "format"

-- Settings
mem.aggressive    = true
mem.whiteknight   = true
mem.formation     = "echelon_right"

local bribe_no_list = {
   _([["Your foul money will not sway House O'rez!"]]),
   _([["You are on the wrong side of history."]]),
}
local taunt_list = {
   _("For House O'rez!"),
   _("Eliminate the traitors!"),
   _("En garde!"),
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

   -- Finish up creation
   create_post()
end

-- When hailed
function hail ()
   local p = ai.pilot()

   -- Remove randomness from future calls
   if not mem.hailsetup then
      mem.refuel_base = rnd.rnd( 2000, 4000 )
      mem.bribe_base = math.sqrt( p:mass() ) * (300 * rnd.rnd() + 850)
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
   local standing = p:reputation()
   mem.refuel = mem.refuel_base
   if standing > 60 then mem.refuel = mem.refuel * 0.7 end
   mem.refuel_msg = fmt.f( _([["I could do you the favour of refueling for the price of {credits}."]]),
         {credits=fmt.credits(mem.refuel)} )

   -- Bribing
   mem.bribe_no = bribe_no_list[ rnd.rnd(1,#bribe_no_list) ]
end

-- taunts
function taunt( _target, _offense )
   -- Offense is not actually used
   local taunts = taunt_list
   return taunts[ rnd.rnd(1,#taunts) ]
end
