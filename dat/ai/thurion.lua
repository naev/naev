require 'ai.core.core'
local fmt = require "format"

mem.shield_run    = 20
mem.armour_run    = 20
mem.defensive     = true
mem.enemyclose    = 500
mem.whiteknight   = true
mem.formation     = "cross"

local sos_msg_list = {
   _("Local security: requesting assistance!"),
   _("Requesting assistance. We are under attack!"),
   _("Vessel under attack! Requesting help!"),
   _("Help! Ship under fire!"),
   _("Taking hostile fire! Need assistance!"),
   _("We are under attack, require support!"),
   _("Mayday! Ship taking damage!"),
   _("01010101011011100110010001100101011100100010000001100001011101000111010001100001011000110110101100100001"), -- "Under attack!" in binary
}
local bribe_no_list = {
   _([["The Thurion will not be bribed!"]]),
   _([["I have no use for your money."]]),
   _([["Credits are no replacement for a good shield."]])
}

-- Sends a distress signal which causes faction loss
local function sos ()
   ai.settarget( ai.taskdata() )
   ai.distress( sos_msg_list[ rnd.rnd(1,#sos_msg_list) ])
end

-- Must be defined after sos
mem.distressmsgfunc = sos

function create ()
   local price = ai.pilot():ship():price()

   -- See if it's a transport ship
   mem.istransport = seeIfTransport()

   -- Credits, and other transport-specific stuff
   if mem.istransport then
      transportParam( price )
   else
      ai.setcredits( rnd.rnd(price/300, price/70) )
   end

   create_post()
end

function hail ()
   -- Remove randomness from future calls
   if not mem.hailsetup then
      mem.refuel_base = rnd.rnd( 1000, 3000 )
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

   -- Refuel
   mem.refuel = mem.refuel_base
   mem.refuel_msg = fmt.f(_([["I'll supply your ship with fuel for {credits}."]]),
         {credits=fmt.credits(mem.refuel)})

   -- No bribe
   mem.bribe_no = bribe_no_list[ rnd.rnd(1,#bribe_no_list) ]
end
