require("ai/tpl/generic")
require("ai/personality/patrol")
require "numstring"

-- Settings
mem.armour_run = 0
mem.armour_return = 0
mem.aggressive = true


function create ()

   -- Not too many credits.
   ai.setcredits( rnd.rnd(ai.pilot():ship():price()/200, ai.pilot():ship():price()/50) )

   -- Get refuel chance
   p = player.pilot()
   if p:exists() then
      standing = ai.getstanding( p ) or -1
      mem.refuel = rnd.rnd( 1000, 2000 )
      if standing < 70 then
         mem.refuel_no = _("\"I do not have fuel to spare.\"")
      else
         mem.refuel = mem.refuel * 0.6
      end
      -- Most likely no chance to refuel
      mem.refuel_msg = string.format( _("\"I would be able to refuel your ship for %s.\""), creditstring(mem.refuel) )
   end

   -- Can't be bribed
   bribe_no = {
          _("\"Your money is of no interest to me.\"")
   }
   mem.bribe_no = bribe_no[ rnd.rnd(1,#bribe_no) ]

   mem.loiter = 2 -- This is the amount of waypoints the pilot will pass through before leaving the system

   -- Finish up creation
   create_post()
end

-- taunts
function taunt ( target, offense )

   -- Only 50% of actually taunting.
   if rnd.rnd(0,1) == 0 then
      return
   end

   -- some taunts
   if offense then
      taunts = {
            _("The universe shall be cleansed of your presence!")
      }
   else
      taunts = {
            _("Sirichana protect me!"),
            _("You have made a grave error!"),
            _("You do wrong in your provocations!")
      }
   end

   ai.pilot():comm(target, taunts[ rnd.rnd(1,#taunts) ])
end


