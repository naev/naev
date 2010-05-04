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
      beacons[k]:control()
      beacons[k]:setHostile() --hostile/friendly to indicate next beacon
      beacons[k]:setInvincible()
   end
   
   
   -- spawn participants here
   racers = {}
   racers[1] = racer:new( pilot.player(), "player", 1, beacons)
   --racer[s2] = racer.new( pilot.add("Goddard Goddard"), "player", 2, beacons)
   
   --pilot.player():setPos( beacon_pos[beacons_done] ) put player to start beacon
   --x, y = beacon_pos[beacons_done + 1, beacons]:get() direct player towards next beacon. beaconSanity makes sure that only existing beacons can be referenced
   --pilot.player():setDir( math.deg( math.atan( y/x ) ) )
   --pilot.player():disable()
   

   beacons[ beaconSanity( racers[1].beacons_done + 1, beacons ) ]:setFriendly() --this indicates the next beacon, only for player use   
   player.msg( text[3] )
   
   countdown_timer = misn.timerStart( "countdownThree", 3000 ) --start countdown
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
   
   racers[1].pilot:setHealth( 100, 100 ) --enable the player
   
   check_timer = misn.timerStart( "checkProximity", 250 ) --start checking if the player has reached beacons
end

function checkProximity ()
   if vec2.dist( racers[1].pilot:pos(), beaconSanity( beacon_pos[racers[1].beacons_done +1], beacons ) ) <= 100 then
      racers[1].beacons_done = racers[1].beacons_done + 1
      
      if racers[1].beacons_done % #beacons == 0 then --when round is completed
         player.msg( string.format( text[8], rounds - rounds * #beacons / racers[1].beacons_done ) )
         
         if racers[1].rounds_completed == rounds * #beacons then --when race is completed
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
   misn.finish(false)
end