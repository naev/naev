require("ai/tpl/generic.lua")
require("ai/personality/patrol.lua")

-- Settings
mem.armour_run = 40
mem.armour_return = 70
mem.aggressive = true


function create ()

   ai.setcredits( rnd.int(ai.pilot():ship():price()/150, ai.pilot():ship():price()/50) )

   if rnd.int() > 0.7 then
      mem.bribe = math.sqrt( ai.pilot():stats().mass ) * (750. * rnd.int() + 2500.)
      mem.bribe_prompt = string.format(_("\"Your life is worth %d credits to me.\""), mem.bribe )
      mem.bribe_paid = _("\"Beat it.\"")
   else
      if rnd.int() > 0.5 then
         mem.bribe_no = _("\"You won't buy your way out of this one.\"")
      else
         mem.bribe_no = _("\"I'm afraid you can't make it worth my while.\"")
      end
   end

   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system

   -- Finish up creation
   create_post()
end

-- taunts
function taunt ( target, offense )

   -- Only 20% of actually taunting.
   if rnd.int(0,4) ~= 0 then
      return
   end

   -- some taunts
   if offense then
      taunts = {
            _("Don't take this personally."),
            _("It's just business.")
      }
   else
      taunts = {
            _("Your skull will make a great hood ornament."),
            _("I've destroyed ships twice the size of yours!"),
            _("I'll crush you like a grape!"),
            _("This isn't what I signed up for!")
      }
   end

   ai.pilot():comm(target, taunts[ rnd.int(1,#taunts) ])
end


