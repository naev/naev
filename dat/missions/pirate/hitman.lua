--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Hitman">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>2</priority>
  <chance>10</chance>
  <location>Bar</location>
  <cond>system.cur() == system.get("Alteris")</cond>
 </avail>
 <notes>
  <tier>3</tier>
 </notes>
</mission>
--]]
--[[

   Pirate Hitman

   Corrupt Merchant wants you to destroy competition

   Author: nloewen

--]]
local fmt = require "format"
local pir = require "common.pirate"

-- Mission constants
local targetsystem = system.get("Delta Pavonis")

local attack_finished -- Forward-declared functions

function create ()
   -- Note: this mission does not make any system claims.
   misn.setNPC( _("Shifty Trader"),  "neutral/unique/shifty_merchant.webp", _("You see a shifty looking man sitting in a darkened corner of the bar. He is trying to discreetly motion you to join him, but is only managing to make himself look suspicious. Perhaps he's watched too many holovideos."))
end


--[[
Mission entry point.
--]]
function accept ()
   -- Mission details:
   if not tk.yesno( _("Spaceport Bar"), _([[The man motions for you to take a seat next to him. Voice barely above a whisper, he asks, "How'd you like to earn some easy money? If you're comfortable with getting your hands dirty, that is."]]) ) then
      misn.finish()
   end
   misn.accept()

   -- Some variables for keeping track of the mission
   mem.misn_done = false
   mem.fledTraders = 0
   mem.misn_base, mem.misn_base_sys = planet.cur()

   -- Set mission details
   misn.setTitle( _("Thug") )
   misn.setReward( _("Some easy money") )
   misn.setDesc( fmt.f( _("A shifty businessman has tasked you with chasing away merchant competition in the {sys} system."), {sys=targetsystem} ) )
   mem.misn_marker = misn.markerAdd( targetsystem, "low" )
   misn.osdCreate( _("Thug"), {
      fmt.f(_("Attack, but do not kill, Trader pilots in the {sys} system so that they run away"), {sys=targetsystem} ),
      fmt.f(_("Return to {pnt} in the {sys} system for payment"), {pnt=mem.misn_base, sys=mem.misn_base_sys} ),
   } )
   -- Some flavour text
   tk.msg( _("Spaceport Bar"), fmt.f( _([[Apparently relieved that you've accepted his offer, he continues, "There're some new merchants edging in on my trade routes in {sys}. I want you to make sure they know they're not welcome." Pausing for a moment, he notes, "You don't have to kill anyone, just rough them up a bit."]]), {sys=targetsystem}) )

   -- Set hooks
   hook.enter("sys_enter")
end

-- Entering a system
function sys_enter ()
   mem.cur_sys = system.cur()
   -- Check to see if reaching target system
   if mem.cur_sys == targetsystem then
      hook.pilot(nil, "attacked", "trader_attacked")
   end
end

-- Attacked a trader
function trader_attacked (hook_pilot, hook_attacker, _arg)
   if mem.misn_done then
      return
   end

   if ( hook_pilot:faction() == faction.get("Trader")
            or hook_pilot:faction() == faction.get("Traders Guild") )
         and ( hook_attacker == player.pilot()
            or hook_attacker:leader() == player.pilot() ) then
      hook_pilot:hookClear()
      hook.pilot(hook_pilot, "jump", "trader_jumped")
      hook.pilot(hook_pilot, "land", "trader_jumped")
   end
end

-- An attacked Trader Jumped
function trader_jumped (_pilot, _arg)
   if mem.misn_done then
      return
   end

   mem.fledTraders = mem.fledTraders + 1
   if mem.fledTraders >= 5 then
      attack_finished()
   end
end

-- attack finished
function attack_finished()
   if mem.misn_done then
      return
   end
   mem.misn_done = true
   player.msg( _("MISSION SUCCESS! Return for payment.") )
   misn.markerRm( mem.misn_marker )
   mem.misn_marker = misn.markerAdd( mem.misn_base_sys, "low" )
   misn.osdActive(2)
   hook.land("landed")
end

-- landed
function landed()
   if planet.cur() == mem.misn_base then
      tk.msg(_("Mission Complete"), _([[As you inform your acquaintance that you successfully scared off the traders, he grins and transfers a sum of credits to your account. "That should teach them to stay out of my space."]]))
      player.pay(150e3)
      pir.modDecayFloor(2)
      pir.modReputation(2)
      faction.modPlayerSingle("Pirate", 5)
      pir.addMiscLog(_([[You chased away a shifty merchant's competition and were paid a sum of credits by the shifty merchant for your services.]]))
      misn.finish(true)
   end
end
