--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Hitman 2">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>2</priority>
  <chance>10</chance>
  <location>Bar</location>
  <cond>system.cur() == system.get("Alteris")</cond>
  <done>Hitman</done>
 </avail>
</mission>
--]]
--[[

   Pirate Hitman 2

   Corrupt Merchant wants you to destroy competition

   Author: nloewen

--]]
local fmt = require "format"
local pir = require "common.pirate"

-- Mission constants
local targetsystem = system.get("Delta Pavonis")

local attack_finished -- Forward-declared functions
-- luacheck: globals landed sys_enter trader_death (Hook functions passed by name)

function create ()
   -- Note: this mission does not make any system claims.
   misn.setNPC( _("Shifty Trader"),  "neutral/unique/shifty_merchant.webp", _("You see the shifty merchant who hired you previously. He looks somewhat anxious, perhaps he has more business to discuss."))
end


--[[
Mission entry point.
--]]
function accept ()
   -- Mission details:
   if not tk.yesno( _("Spaceport Bar"), _([[As you approach, the man turns to face you and his anxiousness seems to abate somewhat. As you take a seat he greets you, "Ah, so we meet again. My, shall we say... problem, has recurred." Leaning closer, he continues, "This will be somewhat bloodier than last time, but I'll pay you more for your trouble. Are you up for it?"]]) ) then
      misn.finish()
   end
   misn.accept()

   -- Some variables for keeping track of the mission
   mem.misn_done      = false
   mem.attackedTraders = {}
   mem.deadTraders = 0
   mem.misn_base, mem.misn_base_sys = planet.cur()

   -- Set mission details
   misn.setTitle( _("Assassin") )
   misn.setReward( _("Some easy money") )
   misn.setDesc( fmt.f( _("A shifty businessman has tasked you with killing merchant competition in the {sys} system."), {sys=targetsystem} ) )
   mem.misn_marker = misn.markerAdd( targetsystem, "low" )
   misn.osdCreate( _("Assassin"), {
      fmt.f(_("Kill Trader pilots in the {sys} system"), {sys=targetsystem} ),
      fmt.f(_("Return to {pnt} in the {sys} system for payment"), {pnt=mem.misn_base, sys=mem.misn_base_sys} ),
   } )
   -- Some flavour text
   tk.msg( _("Spaceport Bar"), _([[He nods approvingly. "It seems that the traders are rather stubborn, they didn't get the message last time and their presence is increasing." He lets out a brief sigh before continuing, "This simply won't do, it's bad for business. Perhaps if a few of their ships disappear, they'll take the hint." With the arrangement in place, he gets up. "I look forward to seeing you soon. Hopefully this will be the end of my problems."]]) )

   -- Set hooks
   hook.enter("sys_enter")
end

-- Entering a system
function sys_enter ()
   mem.cur_sys = system.cur()
   -- Check to see if reaching target system
   if mem.cur_sys == targetsystem then
      hook.pilot(nil, "death", "trader_death")
   end
end

-- killed a trader
function trader_death (hook_pilot, hook_attacker, _arg)
   if mem.misn_done then
      return
   end
   local pp = player.pilot()

   if ( hook_pilot:faction() == faction.get("Trader")
            or hook_pilot:faction() == faction.get("Traders Guild") )
         and ( hook_attacker == pp
            or hook_attacker:leader() == pp ) then
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
      tk.msg(_("Mission Complete"), _([[You glance around, looking for your acquaintance, but he has noticed you first, motioning for you to join him. As you approach the table, he smirks. "I hope the Empire didn't give you too much trouble." After a short pause, he continues, "The payment has been transferred. Much as I enjoy working with you, hopefully this is the last time I'll require your services."]]))
      player.pay(500e3)
      pir.modDecayFloor(3)
      pir.modReputation(3)
      faction.modPlayerSingle("Pirate", 5)
      pir.addMiscLog(_([[You assassinated some of the shifty merchant's competition and were paid a sum of credits for your services. He said that he should hopefully not require further services from you.]]))
      misn.finish(true)
   end
end
