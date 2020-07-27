require("dat/ai/tpl/generic.lua")
require("dat/ai/personality/trader.lua")


-- Sends a distress signal which causes faction loss
function sos ()
   msg = {
      _("Mayday! We are under attack!"),
      _("Requesting assistance. We are under attack!"),
      _("Merchant vessel under attack! Requesting help!"),
      _("Help! Ship under fire!"),
      _("Taking hostile fire! Need assistance!"),
      _("We are under attack, require support!"),
      _("Mayday! Ship taking damage!"),
      string.format(_("Mayday! Merchant %s being assaulted!"), string.lower( ai.pilot():ship():class() ))
   }
   ai.settarget( ai.target() )
   ai.distress( msg[ rnd.int(1,#msg) ])
end


mem.shield_run = 100
mem.armour_run = 100
mem.defensive  = false
mem.enemyclose = 500
mem.distressmsgfunc = sos
mem.careful   = true


function create ()

   -- Probably the ones with the most money
   ai.setcredits( rnd.int(ai.pilot():ship():price()/100, ai.pilot():ship():price()/25) )

   -- Communication stuff
   mem.bribe_no = _("\"The Space Traders do not negotiate with criminals.\"")
   mem.refuel = rnd.rnd( 3000, 5000 )
   p = player.pilot()
   if p:exists() then
      standing = ai.getstanding( p ) or -1
      if standing > 50 then mem.refuel = mem.refuel * 0.75
      elseif standing > 80 then mem.refuel = mem.refuel * 0.5
      end
      mem.refuel_msg = string.format(_("\"I'll supply your ship with fuel for %d credits.\""),
            mem.refuel);
   end

   -- Some stuff has more chance then others
   num = rnd.int(12)
   if num < 5 then
      cargo = "Food"
   elseif num < 8 then
      cargo = "Ore"
   elseif num < 10 then
      cargo = "Industrial Goods"
   elseif num < 12 then
      cargo = "Luxury Goods"
   else
      cargo = "Medicine"
   end
   ai.pilot():cargoAdd( cargo, rnd.int(0, ai.pilot():cargoFree() ) )

   -- Finish up creation
   create_post()
end
