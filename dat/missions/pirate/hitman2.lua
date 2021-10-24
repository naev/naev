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
local pir = require "common.pirate"

function create ()
   -- Note: this mission does not make any system claims.
   targetsystem = system.get("Delta Pavonis") -- Find target system

   -- Spaceport bar stuff
   misn.setNPC( _("Shifty Trader"),  "neutral/unique/shifty_merchant.webp", _("You see the shifty merchant who hired you previously. He looks somewhat anxious, perhaps he has more business to discuss."))
end


--[[
Mission entry point.
--]]
function accept ()
   -- Mission details:
   if not tk.yesno( _("Spaceport Bar"), string.format( _([[As you approach, the man turns to face you and his anxiousness seems to abate somewhat. As you take a seat he greets you, "Ah, so we meet again. My, shall we say... problem, has recurred." Leaning closer, he continues, "This will be somewhat bloodier than last time, but I'll pay you more for your trouble. Are you up for it?"]]),
          targetsystem:name() ) ) then
      misn.finish()
   end
   misn.accept()

   -- Some variables for keeping track of the mission
   misn_done      = false
   attackedTraders = {}
   attackedTraders["__save"] = true
   deadTraders = 0
   misn_base, misn_base_sys = planet.cur()

   -- Set mission details
   misn.setTitle( string.format( _("Assassin"), targetsystem:name()) )
   misn.setReward( string.format( _("Some easy money"), credits) )
   misn.setDesc( string.format( _("A shifty businessman has tasked you with killing merchant competition in the %s system."), targetsystem:name() ) )
   misn_marker = misn.markerAdd( targetsystem, "low" )
   local osd_desc = {}
   osd_desc[1] = _("Kill Trader pilots in the %s system"):format( targetsystem:name() )
   osd_desc[2] = _("Return to %s in the %s system for payment"):format( misn_base:name(), misn_base_sys:name() )
   misn.osdCreate( _("Assassin"), osd_desc )
   -- Some flavour text
   tk.msg( _("Spaceport Bar"), string.format( _([[He nods approvingly. "It seems that the traders are rather stubborn, they didn't get the message last time and their presence is increasing." He lets out a brief sigh before continuing, "This simply won't do, it's bad for business. Perhaps if a few of their ships disappear, they'll take the hint." With the arrangement in place, he gets up. "I look forward to seeing you soon. Hopefully this will be the end of my problems."]]), targetsystem:name()) )

   -- Set hooks
   hook.enter("sys_enter")
end

-- Entering a system
function sys_enter ()
   cur_sys = system.cur()
   -- Check to see if reaching target system
   if cur_sys == targetsystem then
      hook.pilot(nil, "death", "trader_death")
   end
end

-- killed a trader
function trader_death (hook_pilot, hook_attacker, _arg)
   if misn_done then
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
   if misn_done then
      return
   end
   misn_done = true
   player.msg( _("MISSION SUCCESS! Return for payment.") )
   misn.markerRm( misn_marker )
   misn_marker = misn.markerAdd( misn_base_sys, "low" )
   misn.osdActive(2)
   hook.land("landed")
end

-- landed
function landed()
   if planet.cur() == misn_base then
      tk.msg(_("Mission Complete"), _([[You glance around, looking for your acquaintance, but he has noticed you first, motioning for you to join him. As you approach the table, he smirks. "I hope the Empire didn't give you too much trouble." After a short pause, he continues, "The payment has been transferred. Much as I enjoy working with you, hopefully this is the last time I'll require your services."]]))
      player.pay(500e3)
      pir.modDecayFloor(3)
      pir.modReputation(3)
      faction.modPlayerSingle("Pirate", 5)
      pir.addMiscLog(_([[You assassinated some of the shifty merchant's competition and were paid a sum of credits for your services. He said that he should hopefully not require further services from you.]]))
      misn.finish(true)
   end
end
