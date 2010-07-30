--[[

   Pirate Hitman 2

   Corrupt Merchant wants you to destroy competition

   Author: nloewen

--]]

-- Localization, choosing a language if naev is translated for non-english-speaking locales.
lang = naev.lang()
if lang == "es" then
else -- Default to English
   -- Bar information
   bar_desc = "The trader that hired you is back. It looks like he might want to talk."

   -- Mission details
   misn_title  = "Pirate Hitman 2"
   misn_reward = "Some easy money." -- Possibly some hard to get contraband once it is introduced
   misn_desc   = {}
   misn_desc[1] = "Take out some merchant competition in the %s system."
   misn_desc[2] = "Return to %s in the %s system for payment."

   -- Text
   title    = {}
   text     = {}
   title[1] = "Spaceport Bar"
   text[1]  = [[Ah, your back. I have some more work for you. Payment will be a bit sweeter as well. Are you interested?]]
   text[2] = [[The merchants haven't left. I'm going to need you to step it up a notch from last time. A few missing traders might send the message.]]
   title[3] = "Mission Complete"
   text[3] = [[I hope the empire didn't give you much trouble. I'm sure that after burner came in handy a few times. *chuckles*]]

   -- Messages
   msg      = {}
   msg[1]   = "MISSION SUCCESS! Return for payment."
end

function create ()
   targetsystem = system.get("Delta Pavonis") -- Find target system

   -- Spaceport bar stuff
   misn.setNPC( "Shifty Trader",  "shifty_merchant")
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
   attackedTraders[1] = 0
   fledTraders = 0
   misn_base, misn_base_sys = planet.cur()

   -- Set mission details
   misn.setTitle( string.format( misn_title, targetsystem:name()) )
   misn.setReward( string.format( misn_reward, credits) )
   misn.setDesc( string.format( misn_desc[1], targetsystem:name() ) )
   misn_marker = misn.markerAdd( targetsystem, "low" )

   -- Some flavour text
   tk.msg( title[1], string.format( text[2], targetsystem:name()) )

   -- Set hooks
   hook.enter("sys_enter")
end

-- Entering a system
function sys_enter ()
   cur_sys = system.get()
   -- Check to see if reaching target system
   if cur_sys == targetsystem then
      hook.pilot(nil, "attacked", "trader_attacked")
      hook.pilot(nil, "death", "trader_death")
   end
end

-- Attacked a trader
function trader_attacked (hook_pilot, hook_attacker, hook_arg)
   if misn_done then
      return
   end

   if hook_pilot:faction() == faction.get("Trader") and hook_attacker == pilot.player() then
      attackedTraders[#attackedTraders + 1] = hook_pilot
   end
end

-- An attacked Trader Jumped
function trader_death (hook_pilot, hook_arg)
   if misn_done then
      return
   end

   for i, array_pilot in pairs(attackedTraders) do
      if array_pilot:exists() then
         if array_pilot == hook_pilot then
            deadTraders = deadTraders + 1
            if deadTraders >= 3 then
               attack_finished()
            end
         end
      end
   end
end

-- attack finished
function attack_finished()
   misn_done = true
   player.msg( msg[1] )
   misn.setDesc( string.format( misn_desc[2], misn_base:name(), misn_base_sys:name() ) )
   misn.markerRm( misn_marker )
   misn_marker = misn.markerAdd( misn_base_sys, "low" )
   hook.land("landed")
end

-- landed
function landed()
   if planet.get() == misn_base then
      tk.msg(title[3], text[3])
      player.pay(25000)
      player.modFaction("Pirate",5)
      misn.finish(true)
   end
end
