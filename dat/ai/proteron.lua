require 'ai.core.core'
local fmt = require "format"

-- Settings
mem.armour_run    = 40
mem.armour_return = 70
mem.aggressive    = true
mem.whiteknight   = true
mem.formation     = "fishbone"

local bribe_no_list = {
   _([["You won't buy your way out of this one."]]),
   _([["We like to make examples out of scum like you."]]),
   _([["You've made a huge mistake."]]),
   _([["Bribery carries a harsh penalty, scum."]]),
   _([["I'm not interested in your blood money!"]]),
   _([["All the money in the world won't save you now!"]])
}
local taunt_list_offensive = {
   _("Animals like you don't deserve to live!"),
   _("Begone from this universe, inferior scum!"),
   _("We will cleanse you and all other scum from this universe!"),
   _("Enemies of the state will not be tolerated!"),
   _("Long live the Proteron!"),
   _("War is peace!"),
   _("Freedom is slavery!"),
}
local taunt_list_defensive = {
   _("How dare you attack the Proteron?!"),
   _("I will have your head!"),
   _("You'll regret that!"),
   _("Your fate has been sealed, dissident!"),
   _("You will pay for your treason!"),
   _("Die along with the old Empire!"),
}

function create ()
   create_pre()

   -- Not too many credits.
   local p = ai.pilot()
   local ps = p:ship()
   local price = ps:price()

   -- See if it's a transport ship
   mem.istransport = ps:tags().transport

   -- Credits, and other transport-specific stuff
   if mem.istransport then
      transportParam( price )
   else
      ai.setcredits( rnd.rnd(price/300, price/70) )
   end

   -- Lines to annoy the player.
   local r = rnd.rnd(0,20)
   if r == 0 then
      p:broadcast(_("The Proteron are watching you."))
   end

   -- Finish up creation
   create_post()
end

function hail ()
   local p = ai.pilot()

   -- Remove randomness from future calls
   if not mem.hailsetup then
      mem.refuel_base = mem.refuel_base or rnd.rnd( 2000, 4000 )
      mem.refuel_rng = rnd.rnd()
      mem.bribe_base = mem.bribe_base or math.sqrt( p:mass() ) * (500 * rnd.rnd() + 1750)
      mem.bribe_rng = rnd.rnd()
      mem.hailsetup = true
   end

   -- Clean up
   mem.refuel        = 0
   mem.refuel_msg    = nil
   mem.bribe         = 0
   mem.bribe_prompt  = nil
   mem.bribe_prompt_nearby = nil
   mem.bribe_paid    = nil
   mem.bribe_no      = nil

   -- Deal with refueling
   local standing = p:faction():playerStanding()
   mem.refuel = mem.refuel_base
   if standing < 0 then
      mem.refuel_no = _([["My fuel isn't for sale."]])
   elseif standing < 50 then
      if mem.refuel_rng > 0.8 then
         mem.refuel_no = _([["Sorry, my fuel isn't for sale."]])
      end
   else
      mem.refuel = mem.refuel * 0.6
   end
   -- Most likely no chance to refuel
   mem.refuel_msg = fmt.f( _([["I can transfer some fuel for {credits}."]]), {credits=fmt.credits(mem.refuel)} )

   -- See if can be bribed
   mem.bribe = mem.bribe_base
   if mem.allowbribe or (mem.natural and mem.bribe_rng > 0.6) then
      mem.bribe_prompt = fmt.f(_([["The Proteron can always use some income. {credits} and you were never here."]]), {credits=fmt.credits(mem.bribe)} )
      mem.bribe_paid = _([["Get lost before I have to dispose of you."]])
   else
      mem.bribe_no = bribe_no_list[ rnd.rnd(1,#bribe_no_list) ]

   end
end

-- taunts
function taunt ( target, offense )

   -- Only 50% of actually taunting.
   if rnd.rnd(0,1) == 0 then
      return
   end

   -- some taunts
   local taunts
   if offense then
      taunts = taunt_list_offensive
   else
      taunts = taunt_list_defensive
   end

   ai.pilot():comm(target, taunts[ rnd.rnd(1,#taunts) ])
end
