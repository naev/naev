require "ai.pirate"
local pirlib = require "ai.core.misc.pirate"

local fmt = require "format"

local taunt_list = {
   _("Rawwwr!"),
   _("Awoooooo!"),
   _("You stand no chance!"),
   _("Ye're about to learn why we're the apex predators of the cosmos!"),
   _("You can run, but ye can't hide from the wrath of the Wild Ones!"),
   _("No mercy for the weak!"),
   _("Prepare to be devoured by the Wild Ones!"),
   _("Ya thought ye could outwit the Wild Ones? Fools!"),
   _("Your demise is written in the stars!"),
   _("Prepare to witness the savage might of the Wild Ones firsthand!"),
   _("Looks like there is some fresh prey!"),
   _("Your demise is imminent!"),
   _("The pack feasts tonight!"),
   _("Ah look, a rabbit caught in the headlights!"),
   _("Looks like a tasty morsel!"),
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
      mem.refuel_base = 0.5 * mem.refuel_base
      mem.bribe_base = 1.2 * mem.bribe_base
      mem.bribe_chance = 0.9
      mem.refuel_standing = 30
   end

   pirlib.hail()
   mem.refuel_msg = fmt.f(_([["The pack has got your back! I'll give you fuel for {credits} a jump!"]]), {credits=fmt.credits(mem.refuel)})
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
