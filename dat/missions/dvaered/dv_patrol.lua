--[[

   Handles random Dvaered Patrol missions.

   Stage 1: Travelling
   Stage 2: Must clear enemies.
   Stage 3: Continue travelling.

]]--

lang = naev.lang()
if lang == "es" then
   -- not translated atm
else -- default english
   misn_desc = {}
   misn_desc[1] = "Patrol %d systems for hostiles: "
   misn_desc[2] = "Travel to the %s system and check for hostiles."
   misn_desc[3] = "Return to %s in the %s system for payment."
   misn_desc[4] = "Clear %s of hostiles."
   misn_reward = "%d credits"
   title = {}
   title[1] = "DV: Routine %d sector patrol"
   title[2] = "DV: Patrol %d sectors"
   title[3] = "DV: Scan of %d sectors"
   accept_title = "Mission Accepted"
   msg_title = {}
   msg_msg = {}
   msg_title[1] = "Mission Success"
   msg_msg[1] = "You are greeted by a Dvaered official and recieve your payment of %d credits for your contribution in keeping Dvaered systems clean."
   msg_msg[2] = "DV: Engage hostiles."
   msg_msg[3] = "MISSION FAILED: Fled from the heat of battle."
   msg_msg[4] = "DV: System clear, continue patrol."
   msg_msg[5] = "DV: Patrol finished, return to base."
end

      

-- Create the mission
function create ()

   -- Get systems to patrol
   num_systems = rnd.int(2,4)
   systems = {}
   s = space.getSystem():adjacentSystems()
   systems[1] = s[rnd.int(1,#s)]
   for i=2, num_systems do
      s = systems[i-1]:adjacentSystems()
      systems[i] = s[rnd.int(1,#s)]
   end
   system1, system2, system3, system4 = unpack(systems)
   base, base_sys = space.getPlanet()
   misn.setMarker(systems[1])

   -- Create the description.
   desc = string.format( misn_desc[1], num_systems ) .. systems[1]:name()
   for i=2, num_systems-1 do
      desc = desc .. ", " .. systems[i]:name()
   end
   desc = desc .. " and " .. systems[num_systems]:name() .. "."

   -- Set mession stage
   misn_stage = 1
   visited = 0

   -- Calculate reward
   reward = 10000
   for i=1, num_systems do
      reward = reward + 15000 + 5000 * rnd.twosigma()
   end

   -- Set some details.
   misn.setTitle( string.format( title[rnd.int(1,3)], num_systems ) )
   misn.setDesc( desc )
   misn.setReward( string.format( misn_reward, reward ) )
end

-- Mission is accepted
function accept ()
   if misn.accept() then

      -- Update the description.
      misn.setDesc( string.format( misn_desc[2], systems[1]:name() ) )

      -- Set the hooks
      hook.land( "land" )
      hook.enter( "jump" )
   end
end

-- Jump hook
function jump ()
   if misn_stage == 1 then
      sys = space.getSystem()

      -- Hack in case it wasn't saved
      if systems == nil then
         systems = { system1, system2, system3, system4 }
      end

      -- Check to see if system is next
      if sys == systems[visited+1] then
         visited = visited + 1

         -- Get the next goal
         setNextGoal()
      end
   elseif misn_stage == 3 then

      player.msg(msg_msg[3])
      misn.finish(false)
   end
end

-- Sets the next goal
function setNextGoal ()
   -- Check to see if there are enemies
   f = faction.get("Dvaered")
   enemies = pilot.get( f:enemies() )
   hostiles = #enemies
   if hostiles > 0 then
      misn_stage = 3

      -- Set hooks
      for k,v in ipairs(enemies) do
         v:setHostile() -- should be hostile to player
         hook.pilot( v, "disable", "death" )
         hook.pilot( v, "jump", "death" )
      end

      -- Update description and send messages
      player.msg(msg_msg[2])
      misn.setDesc( string.format( misn_desc[4], sys:name() ) )

   -- No hostiles, continue route
   else
      -- Finished visiting systems
      if visited >= #systems then
         misn_stage = 2
         player.msg(msg_msg[5])
         misn.setDesc( string.format( misn_desc[3], base:name(), base_sys:name() ) )
         misn.setMarker(base_sys)

      -- Need to visit more systems
      else
         player.msg(msg_msg[4])
         misn.setDesc( string.format( misn_desc[2], systems[visited+1]:name() ) )
         misn.setMarker(systems[visited+1])
      end
   end
end

-- Pilot death hook
function death ()
   hostiles = hostiles - 1
   if hostiles <= 0 then
      misn_stage = 1
      setNextGoal()
   end
end

-- Land hook
function land ()
   landed = space.getPlanet()
   if misn_stage == 2 and landed == base then
      player.pay( reward )
      tk.msg( msg_title[1], string.format( msg_msg[1], reward ))

      -- modify the faction standing
      if player.getFaction("Dvaered") < 70 then
         player.modFaction("Dvaered", rnd.rnd(1, num_systems/2) );
      end

      misn.finish(true)
   end
end

