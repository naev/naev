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

require "missions/pirate/common"


-- Bar information
bar_desc = _("You see a shifty looking man sitting in a darkened corner of the bar. He is trying to discreetly motion you to join him, but is only managing to make himself look suspicious. Perhaps he's watched too many holovideos.")

-- Mission details
misn_title = _("Thug")
misn_reward = _("Some easy money")
misn_desc = _("A shifty businessman has tasked you with chasing away merchant competition in the %s system.")
osd_desc_1 = _("Attack, but do not kill, Trader pilots in the %s system so that they run away")
osd_desc_2 = _("Return to %s in the %s system for payment")

-- Text
title    = {}
text     = {}
title[1] = _("Spaceport Bar")
text[1]  = _([[The man motions for you to take a seat next to him. Voice barely above a whisper, he asks, "How'd you like to earn some easy money? If you're comfortable with getting your hands dirty, that is."]])
title[3] = _("Mission Complete")
text[2] = _([[Apparently relieved that you've accepted his offer, he continues, "There're some new merchants edging in on my trade routes in %s. I want you to make sure they know they're not welcome." Pausing for a moment, he notes, "You don't have to kill anyone, just rough them up a bit."]])
text[3] = _([[As you inform your acquaintance that you successfully scared off the traders, he grins and transfers a sum of credits to your account. "That should teach them to stay out of my space."]])

-- Messages
success_msg = _("MISSION SUCCESS! Return for payment.")

log_text = _([[You chased away a shifty merchant's competition and were paid a sum of credits by the shifty merchant for your services.]])


function create ()
   -- Note: this mission does not make any system claims. 
   targetsystem = system.get("Delta Pavonis") -- Find target system

   -- Spaceport bar stuff
   misn.setNPC( _("Shifty Trader"),  "neutral/unique/shifty_merchant.png")
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
   misn_done = false
   fledTraders = 0
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
      hook.pilot(nil, "attacked", "trader_attacked")
   end
end

-- Attacked a trader
function trader_attacked (hook_pilot, hook_attacker, hook_arg)
   if misn_done then
      return
   end

   if ( hook_pilot:faction() == faction.get("Trader")
            or hook_pilot:faction() == faction.get("Traders Guild") )
         and hook_attacker == player.pilot() then
      hook_pilot:hookClear()
      hook.pilot(hook_pilot, "jump", "trader_jumped")
      hook.pilot(hook_pilot, "land", "trader_jumped")
   end
end

-- An attacked Trader Jumped
function trader_jumped (hook_pilot, hook_arg)
   if misn_done then
      return
   end

   fledTraders = fledTraders + 1
   if fledTraders >= 5 then
      attack_finished()
   end
end

-- attack finished
function attack_finished()
   if misn_done then
      return
   end
   misn_done = true
   player.msg( success_msg )
   misn.markerRm( misn_marker )
   misn_marker = misn.markerAdd( misn_base_sys, "low" )
   misn.osdActive(2)
   hook.land("landed")
end

-- landed
function landed()
   if planet.cur() == misn_base then
      tk.msg(title[3], text[3])
      player.pay(150000)
      pir_modDecayFloor(2)
      pir_modReputation(2)
      faction.modPlayerSingle("Pirate", 5)
      pir_addMiscLog(log_text)
      misn.finish(true)
   end
end
