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
   
   next_beacon = 1 --set to starting beacon
   beacons_done = 0
   
   rounds = 3
   rounds_completed = 0
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

function raceInit ()

   beacon_pos = {} --positions of the beacons (checkpoints)
   beacon_pos[1] = vec2.new( 0, 0 ) --starting beacon, but it doesn't need to be this one
   beacon_pos[2] = vec2.new(580, -270)
   beacon_pos[3] = vec2.new(1690, 340)
   beacon_pos[4] = vec2.new(1450, 1690)
   beacon_pos[5] = vec2.new(0, 970)
   beacon_pos[6] = vec2.new(-420, 1760)
   beacon_pos[7] = vec2.new(-1440, 1090)
   
   
   pilot.clear()
   pilot.toggleSpawn(false)

   --hold_in_place = 1
   
   hook.rm(takeoff_hook)
   hook.jumpout( "forfeit" )
   hook.land( "forfeit" )
   
   beacons = {}
   
   for k, v in pairs(beacon_pos) do
      beacons[k] = pilot.add( "racing Beacon", nil, v )[1]
      beacons[k]:control()
      beacons[k]:setHostile() --hostile/friendly to indicate next beacon
      beacons[k]:setInvincible()
   end
   
   pilot.player():setPos( beacon_pos[next_beacon] ) --put player to start beacon
   nextBeacon()
   x, y = beacon_pos[next_beacon]:get() --direct player towards next beacon
   pilot.player():setDir( math.deg( math.atan( y/x ) ) )
   --holdInPlace()        --make sure the player doesn't move before the start of the race
   pilot.player():disable()
   

   beacons[next_beacon]:setFriendly()
   
   player.msg( text[3] )
   
   countdown_timer = misn.timerStart( "countdownThree", 3000 )
end
   
function forfeit ()
   var.push("race_active", 0)
   misn.finish( false )
end

function nextBeacon () --moves the next beacon one further
   beacons[next_beacon]:setHostile()
   if next_beacon == #beacons then
      next_beacon = 1
   else
      next_beacon = next_beacon + 1
   end
   beacons[next_beacon]:setFriendly()
end

function countdownThree ()
   player.msg( "3" )
   countdown_timer = misn.timerStart( "countdownTwo", 1000 )
end

function countdownTwo ()
   player.msg( "2" )
   countdown_timer = misn.timerStart( "countdownOne", 1000 )
end

function countdownOne ()
   player.msg( "1" )
   countdown_timer = misn.timerStart( "raceStart", 1000 )
end

function raceStart ()
   player.msg( text[4] )
   
   pilot.player():setHealth( 100, 100 ) --enable the player
   
   check_timer = misn.timerStart( "checkProximity", 250 ) --start checking if the player has reached beacons
end

function checkProximity ()
   if vec2.dist( pilot.player():pos(), beacon_pos[next_beacon] ) <= 100 then
      nextBeacon()
 
      beacons_done = beacons_done + 1
      
      if beacons_done == #beacons then --when round is completed
         beacons_done = 0
         rounds_completed = rounds_completed + 1
         
         if rounds_completed == rounds then --when race is completed
            tk.msg( title[2], text[5] )
            var.push( "race_active", 0)
            
            for k, v in pairs(beacons) do --remove the beacons
               v:rm()
            end
            
            pilot.toggleSpawn( true )
            
            misn.finish( true )
            
         else
            player.msg( string.format( text[8], rounds - rounds_completed ) )
         end
         
      end
      
      player.msg( text[2] )
   end
   
   check_timer = misn.timerStart( "checkProximity", 250 ) --no conditional necessary, since mission is deleted upon race completion
end

function abort ()
   for k, v in pairs(beacons) do
      v:rm()
   end
   var.push("race_active", 0)
end

function makeValidAngle ( angle )
   while angle < 0 do
      angle = angle + 360
   end
   while angle >= 360 do
      angle = angle - 360
   end
   return angle
end