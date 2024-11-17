require "ai.pirate"
local pirlib = require "ai.core.misc.pirate"

local fmt = require "format"

local taunt_list = {
   _("The Raven's talons always find their mark!"),
   _("Prepare for a lesson in resource redistribution!"),
   _("Your cargo will fetch a hefty price in the black market!"),
   _("Your precious cargo will line the pockets of the Raven Clan!"),
   _("Time for a profitable encounter!"),
   _("Prepare for a lesson in the art of the deal!"),
   _("Prepare to witness the true power of the market!"),
   _("Your ship is but another prize in the Raven clan's vast portfolio of illicit acquisitions!"),
   _("Your ship's fate was sealed the moment it caught the attention of the Raven clan's calculating eyes!"),
   _("Your fate is sealed by the Raven's hand!"),
   _("The Raven clan will capitalize on your misfortune!"),
   _("Consider this a harsh lesson in economics!"),
   _("Prepare to be plucked clean by the Raven clan!"),
   _("Look! A ship ripe for exploitation!"),
   _("Your demise will be both profitable and satisfying!"),
}

function create ()
   create_pre()
   pirlib.create()
   mem.ambushclose = mem.ambushclose * 0.7 -- Least aggressive
   create_post()

   -- Decent money
   local ps = ai.pilot():ship()
   ai.setcredits( rnd.rnd(ps:price()/50, ps:price()/20) )
   mem.atk_kill = false -- Never kills
end

function hail ()
   pirlib.hail()
   -- Overwrite messages
   mem.refuel_msg = fmt.f(_([["For a transaction of {credits} a jump, I can refuel your ship."]]), {credits=fmt.credits(mem.refuel)})
end

function taunt( target, offense )
   if rnd.rnd() < 0.3 then return end -- Sometimes doesn't taunt

   -- Wild ones always taunt
   if rnd.rnd() < 0.6 then
      ai.pilot():comm(target, taunt_list[ rnd.rnd(1,#taunt_list) ])
   else
      -- Sometimes do a generic taunt
      pirlib.taunt( target, offense )
   end
end
