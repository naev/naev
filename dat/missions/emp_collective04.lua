--[[

   Collective Distraction

   Fifth mission in the collective mini campaign.

   You must go rescue a team of commandos after being attacked by collective
    forces on a surveillence mission.

]]--

lang = naev.lang()
if lang == "es" then
   -- not translated atm
else -- default english
   misn_title = "Collective Distraction"
   misn_reward = "None"
   misn_desc = {}
   misn_desc[1] = "Check for survivors on %s in %s."
   misn_desc[2] = "Travel back to %s in %s."
   title = {}
   title[1] = "Collective Espionage"
   title[2] = "Eiroik"
   title[3] = "Mission Accomplished"
   text = {}
   text[1] = [[As soon as you exit the landing pad you see Sergeant Dimitri waiting for you.  He seems a bit more nervous then usual.
"We haven't heard from the commando team, we have to assume the worst.  It seems like Collective presence has also been incremented around %s.  We need you to go check for survivors.  Would you be willing to embark on another dangerous mission?"]]
   text[2] = [["We'll send extra forces to %s to try to weaken the blockade.  You'll have to fly through and land on %s and see if there are any survivors.  The increased drone patrols will pose an issue, be very careful, this is going to be no walk in the park."]]
   text[3] = [[The atmosphere once again starts giving your shields a workout as you land.  You spend a while flying low until you sensors pick up a reading of possible life forms.  The silhouette of the transport ship is barely.  It seems like they were detected and massacred.  You try to see if you can salvage the readings from their equipment, but it seems like it's completely toasted.
You spend a while searching until you find a datapad on one of the corpses, ignoring the stench of burnt flesh you grab it, just as you hear the sirens go off in your ship.  You have been spotted!  Time to hit the afterburner, you've got one right?]]
   text[4] = [[Sergeant Dimitri's face cannot hide his sadness as he sees you approach with no commando members.
"No survivors eh?  I had that gut feeling.  At least you were able to salvage something?  Good, at least it'll make these deaths not be completely futile.  Meet me in the bar in a while.  We're going to try to process this datapad.  It'll hopefully have the final results."]]
end


function create ()

   misn_target, misn_target_sys = space.getSystem("Eiroik")

   -- Intro text
   if tk.yesno( title[1], string.format(text[1], misn_target:name()) )
      then
      misn.accept()

      misn_stage = 0
      blockade_sys = space.getSystem("NGC-7132")
      misn_base, misn_base_sys = space.getPlanet("Omega Station")
      misn.setMarker(misn_target_sys)

      -- Mission details
      misn.setTitle(misn_title)
      misn.setReward( misn_reward )
      misn.setDesc( string.format(misn_desc[1], misn_target:name(), misn_target_sys:name() ))
      tk.msg( title[1], string.format(text[2], blockade_sys:name(), misn_target:name()) )

      hook.enter("jump")
      hook.land("land")
   end
end

-- Handles jumping to target system
function jump ()
   local sys = space.getSystem()
   local factions = sys:faction()

   -- Create some havoc
   if factions["Collective"] then
      pilot.add("Collective Sml Swarm")
      pilot.add("Collective Sml Swarm")
   elseif sys == blockade_sys then
      pilot.add("Collective Sml Swarm")
      pilot.add("Collective Sml Swarm")
      pilot.add("Empire Sml Defense")
   end
end

-- Handles arrival back to base
function land ()
   local planet = space.getPlanet()

   -- Just landing
   if misn_stage == 0 and planet == misn_target then

      tk.msg( title[2], text[3] )
      misn_stage = 1
      misn.setDesc( string.format(misn_desc[2], misn_base::name(), misn_base_sys:name() ))
      misn.setMarker(misn_base_sys)
      misn_cargo = player.addCargo( "Datapad", 0 )

   elseif misn_stage == 1 and planet == misn_base then

      tk.msg( title[3], text[4] )
      player.rmCargo( misn_cargo )

      -- Rewards
      player.modFaction("Empire",5)

      misn.finish(true)
   end
end
