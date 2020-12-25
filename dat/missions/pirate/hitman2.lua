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

require "missions/pirate/common"


-- Bar information
bar_desc = _("You see the shifty merchant who hired you previously. He looks somewhat anxious, perhaps he has more business to discuss.")

-- Mission details
misn_title  = _("Assassin")
misn_reward = _("Some easy money")
misn_desc = _("A shifty businessman has tasked you with killing merchant competition in the %s system.")
osd_desc_1 = _("Kill Trader pilots in the %s system")
osd_desc_2 = _("Return to %s in the %s system for payment")

-- Text
title    = {}
text     = {}
title[1] = _("Spaceport Bar")
text[1]  = _([[As you approach, the man turns to face you and his anxiousness seems to abate somewhat. As you take a seat he greets you, "Ah, so we meet again. My, shall we say... problem, has recurred." Leaning closer, he continues, "This will be somewhat bloodier than last time, but I'll pay you more for your trouble. Are you up for it?"]])
text[2] = _([[He nods approvingly. "It seems that the traders are rather stubborn, they didn't get the message last time and their presence is increasing." He lets out a brief sigh before continuing, "This simply won't do, it's bad for business. Perhaps if a few of their ships disappear, they'll take the hint." With the arrangement in place, he gets up. "I look forward to seeing you soon. Hopefully this will be the end of my problems."]])
title[3] = _("Mission Complete")
text[3] = _([[You glance around, looking for your acquaintance, but he has noticed you first, motioning for you to join him. As you approach the table, he smirks. "I hope the Empire didn't give you too much trouble." After a short pause, he continues, "The payment has been transferred. Much as I enjoy working with you, hopefully this is the last time I'll require your services."]])

-- Messages
msg      = {}
msg[1]   = _("MISSION SUCCESS! Return for payment.")

log_text = _([[You assassinated some of the shifty merchant's competition and were paid a sum of credits for your services. He said that he should hopefully not require further services from you.]])


function create ()
   -- Note: this mission does not make any system claims. 
   targetsystem = system.get("Delta Pavonis") -- Find target system

   -- Spaceport bar stuff
   misn.setNPC( _("Shifty Trader"),  "neutral/unique/shifty_merchant")
   misn.setDesc( bar_desc )
end


--[[
Mission entry point.
--]]
function accept ()
   -- Mission details:
   if not tk.yesno( title[1], string.format( text[1],
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
   misn.setTitle( string.format( misn_title, targetsystem:name()) )
   misn.setReward( string.format( misn_reward, credits) )
   misn.setDesc( string.format( misn_desc, targetsystem:name() ) )
   misn_marker = misn.markerAdd( targetsystem, "low" )
   local osd_desc = {}
   osd_desc[1] = osd_desc_1:format( targetsystem:name() )
   osd_desc[2] = osd_desc_2:format( misn_base:name(), misn_base_sys:name() )
   misn.osdCreate( misn_title, osd_desc )
   -- Some flavour text
   tk.msg( title[1], string.format( text[2], targetsystem:name()) )

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
function trader_death (hook_pilot, hook_attacker, hook_arg)
   if misn_done then
      return
   end

   if ( hook_pilot:faction() == faction.get("Trader")
            or hook_pilot:faction() == faction.get("Traders Guild") )
         and hook_attacker == player.pilot() then
      attack_finished()
   end
end

-- attack finished
function attack_finished()
   if misn_done then
      return
   end
   misn_done = true
   player.msg( msg[1] )
   misn.markerRm( misn_marker )
   misn_marker = misn.markerAdd( misn_base_sys, "low" )
   misn.osdActive(2)
   hook.land("landed")
end

-- landed
function landed()
   if planet.cur() == misn_base then
      tk.msg(title[3], text[3])
      player.pay(500000) -- 500K
      pir_modDecayFloor(3)
      pir_modReputation(3)
      faction.modPlayerSingle("Pirate", 5)
      pir_addMiscLog(log_text)
      misn.finish(true)
   end
end
