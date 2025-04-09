require "ai.pirate"
local pirlib = require "ai.core.misc.pirate"

local fmt = require "format"

local taunt_list = {
   _("Soul!"),
   _("Prepare to be blissed out by the Dreamers' cosmic vibes!"),
   _("Resistance is futile against the Dreamers' groovy onslaught!"),
   _("Your ship's about to be caught in our cosmic embrace, soul."),
   _("Time to enjoy the groovy spoils of space piracy!"),
   _("Get ready for a wild ride through the cosmic haze!"),
   _("Time to liberate your ship's good vibrations!"),
   _("Your ship's our next trippy adventure!"),
   _("Your ship's sailing through the Dreamers' astral plane now, soul!"),
   _("Time to find your cargo new cosmic connections!"),
   _("Your ship's about to embark on a cosmic trip, soul."),
   _("Time to become one with the cosmos, soul!"),
   _("We'll blow your mind!"),
   _("Let the cosmic vibes flow as we liberate your cargo!"),
   _("Time for a journey of redistribution and good times!"),
   _("Your ship is like, a vessel of abundance!"),
   _("Time to dream!"),
   _("You're in for a nightmare, soul!"),
   _("Time to liberate your cargo hold, soul!"),
   _("Face the mellow might of the Dreamer clan!"),
   _("Looks like you got caught in our groove!"),
   _("Can't you see I'm zoning out, soul."),
}

function create ()
   create_pre()
   pirlib.create()
   mem.ambushclose = mem.ambushclose * 0.85 -- Less aggressive
   mem.atk_skill = 0.75 + 0.25 * rnd.sigma()
   create_post()

   -- Poor
   local ps = ai.pilot():ship()
   ai.setcredits( rnd.rnd(ps:price()/80, ps:price()/40) )
   mem.atk_kill = false -- Never kills
end

function hail ()
   local setup = mem.hailsetup
   pirlib.hailSetup()
   -- Overwrite some options
   if not setup then
      mem.refuel_base = 1.5 * mem.refuel_base
      mem.bribe_base = 0.7 * mem.bribe_base
      mem.bribe_chance = 1.0 -- Always bribable
      mem.refuel_standing = 80
   end

   pirlib.hail()
   mem.refuel_msg = fmt.f(_([["Bummer, soul. I can hook you up with some fuel for {credits} a jump!"]]), {credits=fmt.credits(mem.refuel)})
end

function taunt( target, offense )
   -- Wild ones always taunt
   if rnd.rnd() < 0.6 then
      return taunt_list[ rnd.rnd(1,#taunt_list) ]
   else
      -- Sometimes do a generic taunt
      return pirlib.taunt( target, offense )
   end
end
