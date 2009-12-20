--[[

   Collective Espionage II

   Author: bobbens
      minor edits by Infiltrator

   Third mission in the collective mini campaign.

   You must land on an ex-empire planet in collective territory and return.

]]--

lang = naev.lang()
if lang == "es" then
   -- not translated atm
else -- default english
   bar_desc = "You notice Lt. Commander Dimitri at one of the booths."
   misn_title = "Collective Espionage"
   misn_reward = "None"
   misn_desc = {}
   misn_desc[1] = "Land on %s in the %s system to monitor Collective communications."
   misn_desc[2] = "Travel back to %s in %s."
   title = {}
   title[1] = "Collective Espionage"
   title[2] = "Mission Accomplished"
   text = {}
   text[1] = [[You head over to Lt. Commander Dimitri to see what the results are.
"Hello there again, %s. Bad news on your latest run, you got nothing other than the usual robotic chatter. We'll have to send you out again, but this time we'll follow a different approach. Interested in giving it another shot?"]]
   text[2] = [["On your last run, you were monitoring while out in the open. While you do get better signals, upon noticing your presence, the drones will go into combat mode, and yield only combat transmittions. This mission will consist of hiding and monitoring from a safer spot, hopefully catching them more relaxed."
"When the Collective struck, they quickly took many systems; one of the bigger hits was %s, an important gas giant rich in methane. They destroyed the gas refineries and slaughtered the humans. There was nothing we could do. The turbulence and dense atmosphere there should be able to hide your ship."]]
   text[3] = [["The plan is to have you infiltrate Collective space alone to not arouse too much suspicion. Once inside, you should head to %s in the %s system. Stay low and monitor all frequencies in the system. If anything is suspicious, we'll surely catch it then. Don't forget to make sure you have the four jumps of fuel to be able to get there and back in one piece."
"Good luck, I'll be waiting for you on your return."]]
   text[4] = [[You quickly land on %s and hide in its deep dense methane atmosphere. Your monitoring gear flickers into action, hopefully catching something of some use. With some luck there won't be too many Collective drones when you take off.]]
   text[5] = [[As your ship touches ground, you see Lt. Commander Dimitri come out to greet you.
"How was the weather?" he asks jokingly. "Glad to see you're still in one piece. We'll get right on analysing the data acquired. Those robots have to be up to something. Meet me in the bar later. Meanwhile give yourself a treat; you've earned it. We've made a 100k credit deposit in your bank account. Enjoy it."]]
end


function create ()
   misn.setNPC( "Dimitri", "dimitri" )
   misn.setDesc( bar_desc )
end


function accept ()
   -- Intro text
   if not tk.yesno( title[1], string.format(text[1], player.name()) ) then
      misn.finish()
   end

   -- Accept the mission
   misn.accept()

   misn_stage = 0
   systems_visited = 0 -- Number of Collective systems visited
   misn_base, misn_base_sys = planet.get("Omega Station")
   misn_target, misn_target_sys = planet.get("Eiroik")
   misn.setMarker(misn_target_sys)

   -- Mission details
   misn.setTitle(misn_title)
   misn.setReward( misn_reward )
   misn.setDesc( string.format(misn_desc[1], misn_target:name(), misn_target_sys:name() ))

   tk.msg( title[1], string.format(text[2], misn_target:name()) )
   tk.msg( title[1], string.format(text[3], misn_target:name(), misn_target_sys:name()) )

   hook.land("land")
end

function land()
   pnt = planet.get()

   -- First mission part is landing on the planet
   if misn_stage == 0 and pnt == misn_target then
      -- Sinister music landing
      music.load("landing_sinister")
      music.play()

      -- Some text
      tk.msg( title[1], string.format(text[4], misn_target:name()) )
      misn_stage = 1
      misn.setDesc( string.format(misn_desc[2], misn_base:name(), misn_base_sys:name() ))
      misn.setMarker(misn_base_sys)

   -- Return bit
   elseif misn_stage == 1 and pnt == misn_base then
      tk.msg( title[2], text[5] )

      -- Rewards
      player.modFaction("Empire",5)
      player.pay( 100000 )

      misn.finish(true)
   end
end
