require("dat/ai/tpl/generic.lua")
require("dat/ai/personality/patrol.lua")

-- Settings
mem.armour_run = 40
mem.armour_return = 70
mem.aggressive = true


function create ()

   -- Not too many credits.
   ai.setcredits( rnd.rnd(ai.pilot():ship():price()/300, ai.pilot():ship():price()/70) )

   -- Get refuel chance
   p = player.pilot()
   if p:exists() then
      standing = ai.getstanding( p ) or -1
      mem.refuel = rnd.rnd( 2000, 4000 )
      if standing < 20 then
         mem.refuel_no = _("\"My fuel isn't for sale.\"")
      elseif standing < 70 then
         if rnd.rnd() > 0.2 then
            mem.refuel_no = _("\"Sorry, my fuel isn't for sale.\"")
         end
      else
         mem.refuel = mem.refuel * 0.6
      end
      -- Most likely no chance to refuel
      mem.refuel_msg = string.format( _("\"I can transfer some fuel for %d credits.\""), mem.refuel )
   end

   -- See if can be bribed
   if rnd.rnd() > 0.6 then
      mem.bribe = math.sqrt( ai.pilot():stats().mass ) * (500. * rnd.rnd() + 1750.)
      mem.bribe_prompt = string.format(_("\"House Proteron can always use some income. %d credits and you were never here.\""), mem.bribe )
      mem.bribe_paid = _("\"Get lost before I have to dispose of you.\"")
   else
     bribe_no = {
            _("\"You won't buy your way out of this one.\""),
            _("\"House Proteron likes to make examples out of scum like you.\""),
            _("\"You've made a huge mistake.\""),
            _("\"Bribery carries a harsh penalty, scum.\""),
            _("\"I'm not interested in your blood money!\""),
            _("\"All the money in the world won't save you now!\"")
     }
     mem.bribe_no = bribe_no[ rnd.rnd(1,#bribe_no) ]
     
   end

   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system

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
            _("There is no room in this universe for scum like you!"),
            _("House Proteron will enjoy your death!"),
            _("Your head will make a fine addition to my collection!"),
            _("None survive the wrath of House Proteron!"),
            _("Enjoy your last moments, criminal!")
      }
   else
      taunts = {
            _("You dare attack me!"),
            _("You are no match for House Proteron!"),
            _("The Empire will have your head!"),
            _("You'll regret that!"),
            _("That was a fatal mistake!")
      }
   end

   ai.pilot():comm(target, taunts[ rnd.rnd(1,#taunts) ])
end


