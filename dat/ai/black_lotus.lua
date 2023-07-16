require "ai.pirate"
local pirlib = require "ai.core.misc.pirate"

local fmt = require "format"

local taunt_list = {
   _("Look, a business opportunity!"),
   _("Your fate is sealed, marked by the Black Lotus!"),
   _("Prepare to be indebted to the Black Lotus!"),
   _("The Black Lotus always collects, whether it be in riches or in blood!"),
   _("Looks like your ship is our next lucrative acquisition!"),
   _("The Black Lotus clan takes what it desires."),
   _("Your ship will join the ranks of our conquered vessels."),
   _("We're here to collect our dues."),
   _("We'll show you the true meaning of futility"),
   _("Consider this a stroke of unfortunate luck, for you not us!"),
   _("The Black Lotus always gets what it wants, one way or another."),
   _("Kneel before the might of the Black Lotus!"),
   _("The Black Lotus always finds what it seeks."),
   _("Looks like you've come to the wrong sector!"),
   _("Never cross paths with the Black Lotus, unless you're prepared to pay the price!"),
   _("The Black Lotus claims its rightful share of the spoils!"),
   _("Your demise will be merely a footnote in our tale of power!"),
   _("Resistance is futile against the iron grip of the Black Lotus!"),
}

function create ()
   create_pre()
   pirlib.create()
   create_post()
end

function hail ()
   local setup = mem.hailsetup
   pirlib.hailSetup()
   -- Overwrite some options
   if not setup then
      -- Generally more expensive option
      mem.refuel_base = 1.2 * mem.refuel_base
      mem.bribe_base = 1.3 * mem.bribe_base
      mem.bribe_chance = 1.0 -- Always can bribe
      mem.refuel_standing = 20
   end

   pirlib.hail()
   mem.refuel_msg = fmt.f(_([["In this economy I guess I could spare some fuel for {credits} a jump."]]), {credits=fmt.credits(mem.refuel)})
end

function taunt( target, offense )
   -- Wild ones always taunt
   if rnd.rnd() < 0.6 then
      ai.pilot():comm(target, taunt_list[ rnd.rnd(1,#taunt_list) ])
   else
      -- Sometimes do a generic taunt
      pirlib.taunt( target, offense )
   end
end
