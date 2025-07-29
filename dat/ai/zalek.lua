require 'ai.core.core'
local fmt = require "format"

-- We’ll consider the Za'lek prefer to turn a bad (i.e. battle) situation into
-- a profitable one by getting money and selling fuel if possible if the player
-- hasn’t been too hostile in the past.

-- Settings
mem.armour_run    = 75 -- Za'lek armour is pretty crap. They know this, and will dip when their shields go down.
mem.aggressive    = true
mem.whiteknight   = true
mem.formation     = "circle"

local bribe_no_list = {
   _([["Keep your cash, you troglodyte."]]),
   _([["Don't make me laugh. Eat laser beam!"]]),
   _([["My drones aren't interested in your ill-gotten gains and neither am I!"]]),
   _([["Ahaha! Nice one! Oh, you're actually serious? Ahahahaha!"]]),
   _([["While I admire the spirit of it, testing my patience is suicide, NOT science."]])
}

local bribe_no_list_drone = {
   _([["INSUFFICIENT PERMISSIONS TO TERMINATE COMBAT PROTOCOLS."]]),
   _([["INVALID USER CREDENTIALS."]]),
   function () return fmt.f(_([["'{player}' IS NOT IN THE SUDOERS FILE. THIS INCIDENT WILL BE REPORTED."]]),{player=player.name()}) end,
}

local taunt_list_offensive = {
   _("Move drones in to engage. Cook this clown!"),
   _("Say hello to my little friends!"),
   _("Ooh, more victi- ah, volunteers for our experiments!"),
   _("We need a test subject to test our attack on; you'll do nicely!"),
   _("Ready for a physics lesson, punk?"),
   _("After we wax you, we can return to our experiments!")
}
local taunt_list_defensive = {
   _("We're being attacked! Prepare defence protocols!"),
   _("You just made a big mistake!"),
   _("Our technology will fix your attitude!"),
   _("You wanna do this? Have it your way.")
}
local taunt_list_offensive_drone = {
   _("ENGAGING OFFENSIVE PROTOCOLS."),
   _("EXTERMINATING HOSTILES."),
   _("COMBAT PROTOCOLS INITIATED."),
}
local taunt_list_defensive_drone = {
   _("ENGAGING DEFENSIVE PROTOCOLS."),
}

function create()
   create_pre()
   local p  = ai.pilot()
   local ps = p:ship()
   local pt = ps:tags()
   local price = ps:price()

   -- See if a drone
   mem.isdrone = pt.drone
   if mem.isdrone then
      local msg = _([["ACCESS DENIED."]])
      mem.refuel_no = msg
      mem.bribe_no = msg
      mem.scan_msg = _("COMMENCING SCAN PROCEDURE.")
      mem.scan_msg_ok = _("SCAN COMPLETED.")
      mem.scan_msg_bad = _("ILLEGAL OBJECTS DETECTED! RESISTANCE IS FUTILE!")
      mem.armour_run = 0 -- Drones don't run
      -- Drones can get indirectly bribed as part of fleets
      mem.bribe = math.sqrt( p:mass() ) * (500 * rnd.rnd() + 1750)
      -- Smaller faction hits than normal ships
      mem.distress_hit = mem.distress_hit * 0.5
      create_post()
      return
   end

   -- See if it's a transport ship
   mem.istransport = pt.transport

   -- Credits, and other transport-specific stuff
   if mem.istransport then
      transportParam( price )
   else
      ai.setcredits( rnd.rnd(price/200, price/50) )
   end

   -- Set how far they attack
   mem.enemyclose = 3000 + 1000 * ps:size()
   mem.atk_skill  = 0.75 + 0.25*rnd.sigma()

   -- Finish up creation
   create_post()
end

function hail ()
   local p = ai.pilot()

   -- Remove randomness from future calls
   if not mem.hailsetup then
      mem.refuel_base = mem.refuel_base or rnd.rnd( 2000, 4000 )
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
   local standing = p:reputation()
   mem.refuel = mem.refuel_base
   if standing < -10 then
      if mem.isdrone then
         mem.refuel_no = _([["INSUFFICIENT PERMISSIONS FOR REFUEL."]])
      else
         mem.refuel_no = _([["I do not have fuel to spare."]])
      end
   else
      mem.refuel = mem.refuel * 0.6
   end
   -- Most likely no chance to refuel
   if mem.isdrone then
      mem.refuel_msg = _([["INSERT {credits} TO COMMENCE REFUELLING OPERATIONS."]])
   else
      mem.refuel_msg = _([["I will agree to refuel your ship for {credits}."]])
   end

   -- See if can be bribed
   mem.bribe = mem.bribe_base
   if mem.found_illegal or mem.allowbribe or (mem.natural and (standing > 0 or
         (standing > -20 and mem.bribe_rng > 0.8) or
         (standing > -50 and mem.bribe_rng > 0.6) or
         (rnd.rnd() > 0.4))) then
      if mem.isdrone then
         mem.bribe_prompt = _([["INSERT {credits} TO CEASE HOSTILITIES."]])
         mem.bribe_paid = _([["PEACE PROTOCOL INITIATED."]])
      else
         mem.bribe_prompt = _([["We will agree to end the battle for {credits}."]])
         mem.bribe_paid = _([["Temporarily stopping fire."]])
      end
   else
      if mem.isdrone then
         mem.bribe_no = bribe_no_list_drone[ rnd.rnd(1,#bribe_no_list_drone) ]
      else
         mem.bribe_no = bribe_no_list[ rnd.rnd(1,#bribe_no_list) ]
      end
   end
end

function taunt( _target, offense )
   -- Only 30% of actually taunting.
   if rnd.rnd() > 0.3 then
      return
   end

   local taunts
   if offense then
      taunts = (mem.isdrone and taunt_list_offensive_drone) or taunt_list_offensive
   else
      taunts = (mem.isdrone and taunt_list_defensive_drone) or taunt_list_defensive
   end

   return taunts[ rnd.rnd(1,#taunts) ]
end
