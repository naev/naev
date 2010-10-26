--[[

   Collective Distraction

   Author: bobbens
      minor edits by Infiltrator

   Fifth mission in the collective mini campaign.

   You must go rescue a team of commandos after being attacked by collective
    forces on a surveillance mission.

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
   text[1] = [[As soon as you exit the landing pad you see Lt. Commander Dimitri waiting for you. He seems a bit more nervous then usual.
"We haven't heard from the commando team, so we have to assume the worst. On top of that, it appears that Collective presence has increased around %s. We need you to go check for survivors. Would you be willing to embark on another dangerous mission?"]]
   text[2] = [["We'll send extra forces to %s to try to give you a chance to break through the blockade. You'll have to fly through and land on %s and see if there are any survivors. The increased drone patrols will pose an issue. Be very careful. This is going to be no walk in the park."]]
   text[3] = [[The atmosphere once again starts giving your shields a workout as you land. You spend a while flying low until your sensors pick up a reading of possible life forms. The silhouette of the transport ship is barely visible. As you fly closer, it becomes apparent that they were detected and massacred. You see if you can salvage the readings from their equipment, but it seems like it's completely toasted.]]
   text[4] = [[You notice you won't have enough fuel to get back so you salvage some from the wrecked transport ship. Stealing from the dead isn't pleasant business, but if it gets you out alive, you figure it's good enough.]]
   text[5] = [[You spend a while searching until you find a datapad on one of the corpses. Ignoring the stench of burnt flesh you grab it, just as you hear the sirens go off in your ship. You have been spotted!  Time to hit the afterburner.
   You've got one, right?]]
   text[6] = [[Lt. Commander Dimitri's face cannot hide his sadness as he sees you approach with no commando members.
"No survivors, eh? I had that gut feeling. At least you were able to salvage something? Good, at least it'll make their deaths not be completely futile. Meet me in the bar in a while. We're going to try to process this datapad. It'll hopefully have the final results."]]
end


function create ()
   -- Note: this mission does not make any system claims.

   misn_target, misn_target_sys = planet.get("Eiroik")

   -- Intro text
   if tk.yesno( title[1], string.format(text[1], misn_target:name()) )
      then
      misn.accept()

      misn_stage = 0
      blockade_sys = system.get("Hades")
      misn_base, misn_base_sys = planet.get("Omega Station")
      misn_marker = misn.markerAdd( misn_target_sys, "low" )

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
   local sys = system.cur()
   local factions = sys:faction()

   -- Create some havoc
   if misn_stage == 1 and sys == misn_target_sys then
      d = rnd.rnd( 900, 1200 )
      a = rnd.rnd() * 2 * math.pi
      swarm_position = vec2.new( d*math.cos(a), d*math.sin(a) )
      pilot.add("Collective Sml Swarm", nil, swarm_position)
      hook.timer(3000, "reinforcements")
      hook.timer(9000, "drone_incoming")
   elseif factions[ "Collectvie" ] and factions[ "Collective" ] > 200 then
      pilot.add("Collective Sml Swarm")
      pilot.add("Collective Sml Swarm")
   elseif sys == blockade_sys then
      pilot.add("Collective Sml Swarm")
      pilot.add("Collective Sml Swarm")
      pilot.add("Collective Sml Swarm")
      pilot.add("Empire Sml Defense")
      pilot.add("Empire Sml Defense")
   end
end

-- Creates reinforcements
function reinforcements ()
   player.msg("Reinforcements are here!")
   pilot.add("Empire Med Attack")
end


-- Creates more drones
function drone_incoming ()
   local sys = system.cur()
   if sys == misn_target_sys then -- Not add indefinately
      pilot.add("Collective Sml Swarm")
      hook.timer(9000, "drone_incoming")
   end
end

-- Handles arrival back to base
function land ()
   local pnt = planet.get()

   -- Just landing
   if misn_stage == 0 and pnt == misn_target then

      -- Sinister music landing
      music.load("landing_sinister")
      music.play()

      -- Some flavour text
      tk.msg( title[2], text[3] )

      -- Add fuel if needed
      if player.fuel() < 200 then
         player.refuel(200)
         tk.msg( title[2], text[4] )
      end

      -- Stage 1 fight!
      tk.msg( title[2], text[5] )
      misn_stage = 1
      misn.setDesc( string.format(misn_desc[2], misn_base:name(), misn_base_sys:name() ))
      misn.markerMove( misn_marker, misn_base_sys )

      -- Add goods
      misn_cargo = misn.cargoAdd( "Datapad", 0 )

   elseif misn_stage == 1 and pnt == misn_base then

      tk.msg( title[3], text[6] )
      misn.cargoRm( misn_cargo )

      -- Rewards
      player.modFaction("Empire",5)

      misn.finish(true)
   end
end
