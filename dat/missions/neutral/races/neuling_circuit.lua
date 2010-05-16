--[[
    The first racing game prototype: no ships, no ai, no weapons, just the race itself.
]]--

lang = naev.lang()
if lang == "es" then
else
   npc_name = "Neuling circuit"
   npc_desc = "Even the greatest names among racers have begun their careers small, with races like the Neuling circuit. It is held continuosly and considered low-risk. However, the winnings are also not very high."
   title = {}
   text = {}
   title[1] = "Go for it!" --add further description here when features are done
   title[2] = "Finish!"
   text[1] = "The track is laid out, simply take off and you'll be at the start. Do you want to participate?"
   text[2] = "Checkpoint!"
   text[3] = "Get ready!"
   text[4] = "Go!"
   text[5] = "You're done!"
   text[6] = "Nothing" --reward (add a reward when the system is better)
   text[7] = "Complete the circuit."
   text[8] = "%s rounds to go."
end

function create () --NOTE: do not define any tables in create or accept, since they are not saved. Create them later, so they are re-initialized after loading
   misn.setNPC( npc_name, "race01") --NPC is a picture of the circuit
   misn.setDesc( npc_desc )
end

function accept ()
   if not tk.yesno( title[1], text[1] ) then --if accepted
      misn.finish()
   end
   misn.accept()
   var.push("race_active", 1)
    
   misn.setTitle( npc_name )
   misn.setDesc( text[7] )
   misn.setReward( text[6] )
   
   takeoff_hook = hook.takeoff("raceInit")
end

include("scripts/racing.lua")

function raceInit ()

   beacon_pos = {} --positions of the beacons (checkpoints)
   beacon_pos[1] = vec2.new( 0, 0 )
   beacon_pos[2] = vec2.new(580, -270)
   beacon_pos[3] = vec2.new(1690, 340)
   beacon_pos[4] = vec2.new(1450, 1690)
   beacon_pos[5] = vec2.new(0, 970)
   beacon_pos[6] = vec2.new(-420, 1760)
   beacon_pos[7] = vec2.new(-1440, 1090)
   
   --beacons_done = 1 --NOTE: this is only for the player's beacons.
   
   rounds = 3
   
   pilot.clear()
   pilot.toggleSpawn(false)
   
   hook.rm(takeoff_hook)
   hook.jumpout( "abort" )
   hook.land( "abort" )
   
   beacons = {}
   
   for k, v in pairs(beacon_pos) do
      beacons[k] = pilot.add( "racing Beacon", nil, v )[1]
      beacons[k]:control( true )
      beacons[k]:setHostile() --hostile/friendly to indicate next beacon
      beacons[k]:setInvincible()
   end
   
   
   -- spawn participants here
   racers = {}
   racers[1] = racer:new( nil, "player", 1, beacons )
   racers[2] = racer:new( "Independent Schroedinger", "basic", 2, beacons, "Schroedinger" )
   racers[3] = racer:new( "Independent Hyena", "basic", 3, beacons, "Hyena" )
   racers[4] = racer:new( "Independent Gawain", "basic", 4, beacons, "Gawain" )
   racers[5] = racer:new( "Goddard Goddard", "basic", 5, beacons, "Goddard" )
   racers[6] = racer:new( "Civilian Llama", "basic", 6, beacons, "Llama" )
 
   player.msg( text[3] )
   
   countdown_timer = hook.timer( 3000, "countdownThree"  ) --start countdown
end

function countdownThree ()
   player.msg( "3" )
   countdown_timer = hook.timer( 1000, "countdownTwo"  )
end

function countdownTwo ()
   player.msg( "2" )
   countdown_timer = hook.timer( 1000, "countdownOne" )
end

function countdownOne ()
   player.msg( "1" )
   countdown_timer = hook.timer( 1000, "raceStart" )
end

function raceStart ()
   player.msg( text[4] )
   
   for k,v in pairs(racers) do
      v:startRace( beacons )
   end
   
   check_timer = hook.timer( 250, "controlLoop" ) --start checking if the player has reached beacons
end

function controlLoop ()
   racers_done = 0
   
   positions = {}
   
   for k,v in pairs(racers) do --check for completed beacons / races
      v:checkProx( beacons, rounds ) --does the checking and updates racer.position
      racers_done = racers_done + v.done --amount of racers that are done
      
      local i = 1
      
      while true do
         if positions[ i ] == nil then
            table.insert( positions, i, v )
            break
         elseif v.beacons_done > positions[ i ].beacons_done then
            table.insert( positions, i, v )
            break
         elseif v.beacons_done == positions[ i ].beacons_done and v.distance < positions[ i ].distance then
            table.insert( positions, i, v )
            break
         else
            i = i + 1
         end
      end
   end
   
   placing = "" --convert the order into human readable format...
   for k, v in pairs( positions ) do
      placing = placing .. k .. " - " .. v.name .. "\n"
   end
   misn.setDesc( placing ) --...and display it
   
   if racers_done == #racers then --if everybody is done
      tk.msg( title[2], text[5] )
      for k, v in pairs(beacons) do
         v:rm()
      end
      
      var.push("race_active", 0)
      for k, v in pairs(racers) do
         racers[k] = v:rm()
      end
      misn.finish(true)
   end
   check_timer = hook.timer( 250, "controlLoop" )
end

function abort ()
   if beacons ~= nil then
      for k, v in pairs(beacons) do
         v:rm()
      end
   end
   var.push("race_active", 0)
   if racers ~= nil then
      for k, v in pairs(racers) do
         racers[k] = v:rm()
      end
   end
   pilot.player():setHealth( 100, 100 )
   misn.finish(false)
end
