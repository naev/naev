--[[

   Pirate Hitman

   Corrupt Merchant wants you to destroy competition

   Author: nloewen

--]]

-- Localization, choosing a language if naev is translated for non-english-speaking locales.
lang = naev.lang()
if lang == "es" then
else -- Default to English
   -- Bar information
   bar_desc = "You see a shifty looking guy sitting in the darkest corner of the bar. He is trying to make descreate motions for you to come over but is only managing to look stupid."

   -- Mission details
   misn_title  = "Pirate Hitman"
   misn_reward = "Some easy cash" --Possibly some hard to get contraband once it is introduced
   misn_desc   = "Take out some merchant competition in the %s system."

   -- Text
   title    = {}
   text     = {}
   title[1] = "Spaceport Bar"
   text[1]  = [[How'd you like to earn some easy money.]]
   title[3] = "Mission Compleate"
   text[2] = [[There's some new merchants stealing my trade routs in %s. I want you to let them not welcome. You don't have to kill anyone, just rough them up a bit.]]
   text[3] = [[Did everything go well? Good, good. That should teach them to stay out of my space.]]

   -- Messages
   msg      = {}
   msg[1]   = "MISSION SUCCESS! Return for payment"
end

function create ()
   targetsystem = system.get("Delta Pavonis") --find target system

   -- Spaceport bar stuff
   misn.setNPC( "Shifty Trader",  "shifty_merchant")
   misn.setDesc( bar_desc )

   --some variables for keeping track of the mission
   attackedTraders = 0
   killedTraders = false
   misn_base, misn_base_sys = planet.cur()
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

   -- Set mission details
   misn.setTitle( string.format( misn_title, targetsystem:name()) )
   misn.setReward( string.format( misn_reward, credits) )
   misn.setDesc( string.format( misn_desc, targetsystem:name() ) )
   misn.markerAdd( targetsystem, "low" )

   -- Some flavour text
   tk.msg( title[1], text[2] )

   -- Set hooks
   hook.enter("sys_enter")
end

-- Entering a system
function sys_enter ()
   cur_sys = system.get()
   -- Check to see if reaching target system
   if cur_sys == targetsystem then
      hook.pilot(nil, "attacked", "trader_attacked")
   end
end

-- Attacked a trader
function trader_attacked (hook_pilot, hook_attacker, hook_arg)
   if hook_pilot:faction() == faction.get("Trader") and hook_attacker == pilot.player() then
      attackedTraders = attackedTraders + 1
      if attackedTraders >= 5 then
         attack_finished()
      end
   end
end

-- attack finished
function attack_finished()
   player.msg(msg[1])
   hook.land("landed")
end

-- landed
function landed()
   if planet.get() == misn_base then
      tk.msg(title[3], text[3])
      player.pay(15000)
      player.modFaction("Pirate",5)
      misn.finish(true)
   end
end
