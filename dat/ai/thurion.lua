require 'ai.core.core'
require "numstring"


mem.shield_run = 20
mem.armour_run = 20
mem.defensive  = true
mem.enemyclose = 500
mem.distressmsgfunc = sos

-- Sends a distress signal which causes faction loss
function sos ()
   local msg = {
      _("Local security: requesting assistance!"),
      _("Requesting assistance. We are under attack!"),
      _("Vessel under attack! Requesting help!"),
      _("Help! Ship under fire!"),
      _("Taking hostile fire! Need assistance!"),
      _("We are under attack, require support!"),
      _("Mayday! Ship taking damage!"),
      _("01010101011011100110010001100101011100100010000001100001011101000111010001100001011000110110101100100001") -- "Under attack!" in binary
   }
   ai.settarget( ai.taskdata() )
   ai.distress( msg[ rnd.int(1,#msg) ])
end

function create ()

   -- Credits.
   ai.setcredits( rnd.int(ai.pilot():ship():price()/500, ai.pilot():ship():price()/200) )

   -- No bribe
   local bribe_msg = {
      _("\"The Thurion will not be bribed!\""),
      _("\"I have no use for your money.\""),
      _("\"Credits are no replacement for a good shield.\"")
   }
   mem.bribe_no = bribe_msg[ rnd.int(1,#bribe_msg) ]

   -- Refuel
   mem.refuel = rnd.rnd( 1000, 3000 )
   local p = player.pilot()
   if p:exists() then
      mem.refuel_msg = string.format(_("\"I'll supply your ship with fuel for %s.\""),
            creditstring(mem.refuel))
   end

   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system
   create_post()
end
