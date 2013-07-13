include("dat/ai/tpl/generic.lua")
include("dat/ai/personality/civilian.lua")


mem.shield_run = 20
mem.armour_run = 20
mem.defensive  = true
mem.enemyclose = 500
mem.distressmsgfunc = sos


-- Sends a distress signal which causes faction loss
function sos ()
   msg = {
      "Local security: requesting assistance!",
      "Requesting assistance. We are under attack!",
      "Vessel under attack! Requesting help!",
      "Help! Ship under fire!",
      "Taking hostile fire! Need assistance!",
      "We are under attack, require support!",
      "Mayday! Ship taking damage!",
      "01010101011011100110010001100101011100100010000001100001011101000111010001100001011000110110101100100001" -- "Under attack!" in binary
   }
   ai.settarget( ai.target() )
   ai.distress( msg[ rnd.int(1,#msg) ])
end


function create ()

   -- Credits.
   ai.setcredits( rnd.int(ai.shipprice()/500, ai.shipprice()/200) )

   -- No bribe
   local bribe_msg = {
      "\"The Thurion will not be bribed!\"",
      "\"I have no use for your money.\"",
      "\"Credits are no replacement for a good shield.\""
   }
   mem.bribe_no = bribe_msg[ rnd.int(1,#bribe_msg) ]

   -- Refuel
   mem.refuel = rnd.rnd( 1000, 3000 )
   p = ai.getPlayer()
   if ai.exists(p) then
      standing = ai.getstanding( p ) or -1
      mem.refuel_msg = string.format("\"I'll supply your ship with fuel for %d credits.\"",
            mem.refuel);
   end

   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system
   create_post()
end

