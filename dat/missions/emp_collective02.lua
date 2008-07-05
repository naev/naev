
lang = naev.lang()
if lang == "es" then
   -- not translated atm
else -- default english
   misn_title = "Collective Espionage"
   misn_reward = "None"
   misn_desc = {}
   misn_desc[1] = "Land on %s in the %s system to monitor Collective communications."
   misn_desc[2] = "Travel back to %s in %s."
   title = {}
   title[1] = "Collective Espionage"
   title[2] = "Mission Accomplished"
   text = {}
   text[1] = [[You notice Sargent Dimitry at one of the booths.  You head over to see what the results are.
"Hello again there %s.  Bad news on your latest run, you got nothing other then the usual robotic chatter.  We'll have to send you out again, this time we'll follow a different approach.  Interested in giving it another shot?]]
   text[2] = [["On your last run you were monitoring while out in the open, while you do get better signals, upon noticing your presence.  This mission will consist of hiding and monitoring from a safer spot, hopefully catching them more relaxed."
"When the Collective struck, they quickly took many systems, one of the bigger hits was %s, an important gas giant rich in methane.  They destroyed the gas refineries and slaughtered the humans, there was nothing we could do.  The turbulence and dense atmosphere should be able to hide your ship."]]
   text[3] = [["The plan is to have you infiltrate Collective space alone to not arouse too much suspicion.  Once inside you should head to %s in the %s system.  Stay low and monitor all frequencies in the system.  If anything is suspicious, we'll surely catch it then."
"Good luck, I'll be waiting for you on your return."]]
   text[4] = [[You quickly land on %s and hide in the deep dense methane atmosphere it has.  Your monitoring gear flickers into action, hopefully catching something of use. With some luck there won't be too many Collective ships when you take off.]]
   text[5] = [[As your ship touches ground you see Sargent Dimitry come out to greet you.
"How was the weatherV", he mentions jokingly.  "Glad to see you're in one piece.  We'll get right on analyzing the data acquired.  These robots have to be up to something.  Meet me in the bar later, meanwhile give yourself a treat, you've earned it.   We've made a 100k credit deposit in your bank account, enjoy it."]]
end


function create()
   -- Intro text
   if tk.yesno( title[1], string.format(text[1], player.name()) )
      then
      misn.accept()

      misn_stage = 0      
      systems_visited = 0 -- Number of Collective systems visited
      misn_base, misn_base_sys = space.getPlanet("Omega Station")
      misn_target, misn_target_sys = space.getPlanet("Eiroik")
      misn.setMarker(misn_target_sys)

      -- Mission details
      misn.setTitle(misn_title)
      misn.setReward( misn_reward )
      misn.setDesc( string.format(misn_desc[1], misn_target:name(), misn_target_sys ))

      tk.msg( title[1], string.format(text[2], misn_target:name()) )
      tk.msg( title[1], string.format(text[3], misn_target:name(), misn_target_sys) )

      hook.land("land")
   end
end

function land()
   planet = space.getLanded()

   -- First mission part is landing on the planet
   if misn_stage == 0 and planet == misn_target then
      tk.msg( title[1], string.format(text[4], misn_target:name()) )
      misn_stage = 1
      misn.setMarker(misn_base_sys)

   -- Return bit
   elseif misn_stage == 1 and planet == misn_base then
      tk.msg( title[2], text[5] )

      -- Rewards
      player.modFaction("Empire",5)
      player.pay( 100000 )

      misn.finish(true)
   end
end
