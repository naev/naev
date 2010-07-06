--[[

   Collective Scout

   Author: bobbens
      minor edits by Infiltrator

   Starts the collective mini campaign.

   You must inspect a stray drone.

]]--

lang = naev.lang()
if lang == "es" then
   -- not translated atm
else -- default english
   bar_desc = "You see an Empire Lt. Commander who seems to be motioning you over to the counter."
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
   text[1] = [[You approach the Lt. Commander.
"Hello %s, we have a reconnaissance mission you might be interested in. Commander Soldner said you'd make a good candidate for the mission. You up for the challenge?"]]
   text[2] = [["I don't think we've met. I'm Lt. Commander Dimitri. If all goes well you'll be reporting to me for the next assignments."
"You've heard about the Collective right?  From what we know, the Collective seems to be a sort of 'hive' of robots. They're a recent menace; had the timing to arrive more or less when the Incident occured, otherwise they would have been wiped out by the Emperor's Armada without a sweat. They completely wiped out all human life in Eiroik, and the other worlds they hit. We managed to stop them here, in %s, and constructed this base. Since then it's been more or less a stalemate."]]
   text[3] = [["This week Collective activity has increased heavily around Rockbed. We've been trying to contain them, but reports indicate that a scout broke through. It was last detected near %s, heading east. We expect it to not have gone far. You are to locate the scout and report back to %s in the %s system. It seems like the Collective is planning something and we want to follow their game a little closer."
"It is of vital importance that you do not engage the drone. Report back as soon as you locate it."]]
   text[4] = [[After landing, you head to the Empire military headquarters and find Lt. Commander Dimitri there.
"Well it seems like the drone has some strange fixation on %s. We aren't quite sure what to make of it, but intelligence is working on it. Report back to the bar in a bit and we'll see what we can do about the Collective."]]
   msg_killdrone = "Mission Failed: You weren't supposed to kill the drone!"
end


function create ()
   misn.setNPC( "Lt. Commander", "dimitri" )
   misn.setDesc( bar_desc )
end


function accept ()
   -- Intro text
   if not tk.yesno( title[1], string.format(text[1], player.name()) ) then
      misn.finish()
   end

   -- Accept mission
   misn.accept()

   misn_stage = 0
   misn_nearby = system.get("Coriolis")
   misn_target = system.get("Dune")
   misn_base,misn_base_sys = planet.get("Omega Station")
   misn_marker = misn.markerAdd( misn_nearby, "low" )

   -- Mission details
   misn.setTitle(misn_title)
   misn.setReward( misn_reward )
   misn.setDesc( string.format(misn_desc[1],misn_nearby:name()))

   -- Flavour text and mini-briefing
   tk.msg( title[2], string.format( text[2], misn_base_sys:name() ) )
   tk.msg( title[2], string.format( text[3], misn_nearby:name(),
         misn_base:name(), misn_base_sys:name() ))

   hook.enter("enter")
   hook.land("land")
end


function enter()
   sys = system.get()

   -- additional fleets
   if sys:name() == "Rockbed" then -- increase action for realism
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
      misn.setDesc( string.format(misn_desc[2],misn_base:name(),misn_base_sys:name()) )
      misn_stage = 1
      misn.markerMove( misn_marker, misn_base_sys )
   end
end

function land()
   pnt = planet.get()

   if misn_stage == 1 and  pnt == misn_base then
      tk.msg( title[3], string.format(text[4],misn_target:name()) )
      player.modFaction("Empire",5)
      misn.finish(true)
   end
end

function kill()
   player.msg( msg_killdrone )
   misn.finish(false)
   var.push( "collective_fail", true )
end

