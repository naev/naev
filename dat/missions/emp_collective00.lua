
lang = naev.lang()
if lang == "es" then
   -- not translated atm
else -- default english
   misn_title = "Collective Scout"
   misn_reward = "None"
   misn_desc = {}
   misn_desc[1] = "Find a scout last seen in the %s system."
   misn_desc[2] = "Travel back to %s in %s."
   title = {}
   title[1] = "Empire Officer"
   title[2] = "Briefing"
   title[3] = "Mission Accomplished"
   text = {}
   text[1] = [[As you enter the bar you notice some one signal to you from the counter.  You notice he's wearing an Empire insignia on his uniform.
"Hello %s, we have a reconnaissance you might be interested.  You up for the challenge?"]]
   text[2] = [["I don't think we've met.  I'm Sargent Dimitri.  If all goes well you'll be reporting to me for the next assignments."
"This week Collective activity has increased heavily around NCG-7291.  We've been trying to contain them, but reports detect that a scout broke through.  The scout was last detected near %s.  We expect it to not have gone far.  You are to locate the scout and report back to %s in the %s system.  It seems like the Collective is planning something and we want to follow their game a bit more"
"It is of vital important that you do not engage the drone.  Report back as soon as you locate it."]]
   text[3] = [[After landing you head to the Empire military headquarters and find Sgt. Dimitri there.
"Well it seems like the drone has some strange fixation with %s.  We aren't quite sure what to make of it, but intelligence is on it.  Report back at the bar in bit and we'll see what we can do about the Collective"]]
   msg_killdrone = "MISSION FAILED: You weren't supposed to kill the drone!"
end


function create()
   -- Intro text
   if tk.yesno( title[1], string.format(text[1], player.name()) )
      then
      misn.accept()

      misn_stage = 0
      misn_nearby = "Coriolis"
      misn_target = "Dune"
      misn_base = "Omega Station"
      misn_base_sys = "NCG-7291"
      misn.setMarker(misn_nearby) -- Not exact target

      -- Mission details
      misn.setTitle(misn_title)
      misn.setReward( misn_reward )
      misn.setDesc( string.format(misn_desc[1],misn_nearby))

      -- Flavour text and mini-briefing
      tk.msg( title[2], string.format( text[2], misn_nearby, misn_base, misn_base_sys ))

      hook.enter("enter")
      hook.land("land")
   end
end

function enter()
   sys = space.system()

   -- additional fleets
   if sys == "NCG-7291" then -- increase action for realism
      pilot.add("Empire Sml Defense")
      pilot.add("Collective Sml Swarm")
   elseif sys == misn_target then
      p = pilot.add("Collective Drone", "scout")
      for k,v in pairs(p) do
         hook.pilot( v, "death", "kill")
      end
   end

   -- update mission
   if misn_stage == 0 and sys == misn_target then
      misn.setDesc( string.format(misn_desc[2],misn_base,misn_base_sys) )
      misn_stage = 1
      misn.setMarker(misn_base_sys) -- now we mark return to base
   end
end

function land()
   planet = space.landName()

   if misn_stage == 1 and  planet == misn_base then
      tk.msg( title[3], string.format(text[3],misn_target) )
      player.modFaction("Empire",5)
      misn.finish(true)
   end
end

function kill()
   player.msg( msg_killdrone )
   misn.finish(false)
end

