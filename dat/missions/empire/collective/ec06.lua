--[[

   Operation Cold Metal

   Author: bobbens
      minor edits by Infiltrator

   Seventh and final mission in the Collective Campaign

   Mission Objectives:
      * Assault C-43
      * Final Assault on C-28
         * Kill the Starfire
         * Kill the Trinity (if it got away in Operation Black Metal)

   Stages:
      0) Just started..
      1) Entered C-43.
      2) Cleared C-43.
      3) Entered C-28.
      4) Cleared C-28.
      5) Ran away.

]]--

lang = naev.lang()
if lang == "es" then
   -- not translated atm
else -- default english
   bar_desc = "You see Comodore Keer at a table with a couple of other pilots. She motions you to sit down with them."
   misn_title = "Operation Cold Metal"
   misn_reward = "Fame and Glory"
   misn_desc = {}
   misn_desc[1] = "Neutralize enemy forces in %s."
   misn_desc[2] = "Destroy the Starfire and hostiles in %s."
   misn_desc[3] = "Return to %s in the %s system."
   title = {}
   title[1] = "Bar"
   title[2] = "Operation Cold Metal"
   title[3] = "Mission Success"
   title[4] = "Cowardly Behaviour"
   text = {}
   text[1] = [[You join Commodore Keer's table.
She begins, "We're going to finally attack the Collective. We've gotten the Emperor himself to bless the mission and send some of his better pilots. Would you be interested in joining the destruction of the Collective?"]]
   text[2] = [["The Operation has been dubbed 'Cold Metal'. Our goal is to head to C-00, we'll take the route of %s, %s then C-00. Should we encounter the Starfire at any stage our goal will be to destroy it and head back. We'll also clear each system completely of Collective presence before continuing to the next system. See you in combat, pilots."]]
   text[3] = [[As you do your approach to land on %s you notice big banners placed on the exterior of the station. They seem to be in celebration of the final defeat of the Collective. When you do land you are saluted by the welcoming committee in charge of saluting all the returning pilots.
You notice Commodore Keer. Upon greeting her, she says, "You did a good job out there. No need to worry about the Collective anymore. Without Welsh, the Collective won't stand a chance, since they aren't truly autonomous. Right now we have some ships cleaning up the last of the Collective; shouldn't take too long to be back to normal."]]
   text[4] = [[She continues. "As a symbol of appreciation, you should find a deposit of 500 thousand credits in your account. There will be a celebration later today in the officer's room if you want to join in."

And such ends the Collective threat...]]
   text[5] = [[You recieve a message signed by Commodore Keeras soon as you enter Empire space:
"There is no room for cowards in the Empire's fleet."
The signature does seem valid.]]
   -- Conversation between pilots
   talk = {}
   talk[1] = "System Cleared: Procede to %s."
   talk[2] = "Mission Success: Return to %s."
   talk[3] = "Mission Failure: Return to %s."
end


function create ()
    missys = {system.get("C-43"), system.get("C-28")}
    if not misn.claim(missys) then
        abort()
    end

   misn.setNPC( "Keer", "keer" )
   misn.setDesc( bar_desc )
end


-- Creates the mission
function accept ()

   -- Intro text
   if not tk.yesno( title[1], text[1] ) then
      misn.finish()
   end

   -- Accept the mission
   misn.accept()

   -- Mission data
   misn_stage = 0
   misn_base, misn_base_sys = planet.get("Omega Station")
   misn_target_sys = system.get("C-43")
   misn_final_sys = system.get("C-28")
   misn_marker = misn.markerAdd( misn_target_sys, "high" )

   -- Mission details
   misn.setTitle(misn_title)
   misn.setReward( misn_reward )
   misn.setDesc( string.format(misn_desc[1], misn_target_sys:name() ))

   tk.msg( title[2], string.format( text[2],
         misn_target_sys:name(), misn_final_sys:name() ) )

   hook.jumpout("jumpout")
   hook.enter("jump")
   hook.land("land")
end


function jumpout ()
   last_sys = system.cur()
end


-- Handles jumping to target system
function jump ()
   sys = system.cur()

   if misn_stage == 0 then

      offset = 1000

      -- Entering target system?
      if sys == misn_target_sys then

         -- Create big battle
         enter_vect = player.pos()
         pilot.clear()
         pilot.toggleSpawn(false)
         -- Empire
         emp_fleets = {}
         emp_fleets[1] = "Empire Sml Attack"
         emp_fleets[2] = "Empire Sml Attack"
         emp_fleets[3] = "Dvaered Goddard" -- They help empire
         -- Get position
         x,y = enter_vect:get()
         spawn_vect = enter_vect.new( x, y )
         -- Add pilots
         for k,v in ipairs(emp_fleets) do
            spawn_vect:add( rnd.rnd(-offset,offset), rnd.rnd(-offset,offset) )
            pilots = pilot.add( v, nil, spawn_vect )
            for k,v in ipairs(pilots) do
               v:setFriendly()
            end
         end
         -- Collective
         col_fleets = {}
         col_fleets[1] = "Collective Sml Swarm"
         col_fleets[2] = "Collective Sml Swarm"
         col_fleets[3] = "Collective Sml Swarm"
         -- Set up position
         x,y = enter_vect:get()
         spawn_vect = enter_vect.new( -x, -y )
         -- Count amount created
         col_alive = 0
         for k,v in ipairs(col_fleets) do
            spawn_vect:add( rnd.rnd(-offset,offset), rnd.rnd(-offset,offset) )
            pilots = pilot.add( v, nil, spawn_vect )
            col_alive = col_alive + #pilots
            for k,v in ipairs(pilots) do
               v:setHostile()
               hook.pilot( v, "disable", "col_dead" )
            end
         end

         misn_stage = 1
      end

   elseif misn_stage == 2 then

      -- Entering target system?
      if sys == misn_final_sys then

         -- Create bigger battle
         enter_vect = player.pos()
         pilot.clear()
         pilot.toggleSpawn(false)
         -- Empire
         emp_fleets = {}
         emp_fleets[1] = "Empire Lge Attack"
         emp_fleets[2] = "Empire Med Attack"
         emp_fleets[3] = "Dvaered Goddard" -- They help empire
         -- Get position
         x,y = enter_vect:get()
         spawn_vect = enter_vect.new( x, y )
         -- Add pilots
         for k,v in ipairs(emp_fleets) do
            spawn_vect:add( rnd.rnd(-offset,offset), rnd.rnd(-offset,offset) )
            pilots = pilot.add( v, nil, spawn_vect )
            for k,v in ipairs(pilot) do
               v:setFriendly()
            end
         end
         -- Collective
         col_fleets = {}
         col_fleets[1] = "Starfire"
         col_fleets[2] = "Collective Lge Swarm"
         col_fleets[3] = "Collective Lge Swarm"
         if var.peek("trinity") == true then
            col_fleets[4] = "Trinity"
         end
         -- Set up position
         x,y = enter_vect:get()
         spawn_vect = enter_vect.new( -x, -y )
         -- Add pilots
         col_alive = 0
         for k,v in ipairs(col_fleets) do
            spawn_vect:add( rnd.rnd(-offset,offset), rnd.rnd(-offset,offset) )
            pilots = pilot.add( v, nil, spawn_vect )

            -- Handle special ships
            if v == "Starfire" then
               starfire = pilots[1]
               starfire:setNodisable(true)
            elseif v == "Trinity" then
               trinity = pilots[1]
               trinity:setNodisable(true)
            end

            -- Count amount created
            col_alive = col_alive + #pilots
            for k,v in ipairs(pilots) do
               v:setHostile()
               hook.pilot( v, "disable", "col_dead" )
            end
         end
         misn_stage = 3
      end

   elseif misn_stage == 1 or misn_stage == 3 then

      -- Fled from battle - disgraceful
      misn_stage = 5
      player.msg( string.format( talk[3], misn_base_sys:name() ))

   elseif misn_stage == 5 then

      -- Lower faction by a lot, without making hostile
      f = player.getFaction("Empire") 
      if f > 0 then
         if f > 20 then player.modFactionRaw("Empire", -20)
         else player.modFactionRaw("Empire", -f)
         end
      end

      -- Display message
      tk.msg( title[4], text[5] )

      -- Mission failed
      var.push( "collective_fail", true )
      misn.finish(false)
   end
end


function refuelBroadcast ()
   if refship:exists() then
      refship:broadcast("Tanker in system, contact if in need of fuel.")
      misn.timerStart( "refuelBroadcast", 10000 )
   end
end


function addRefuelShip ()
   -- Create the pilot
   refship = pilot.add( "Trader Mule", "empire_refuel", last_sys )[1]
   refship:rename("Fuel Tanker")
   refship:setFaction("Empire")
   refship:setFriendly()

   -- Maximize fuel
   refship:rmOutfit("all") -- Only will have fuel pods
   local h,m,l = refship:ship():slots()
   refship:addOutfit( "Fuel Pod", l )
   refship:setFuel( true ) -- Set fuel to max

   -- Add some escorts
   refesc = {}
   refesc[1] = pilot.add( "Empire Lancelot", "empire_idle", last_sys )[1]
   refesc[2] = pilot.add( "Empire Lancelot", "empire_idle", last_sys )[1]
   for k,v in ipairs(refesc) do
      v:setFriendly()
   end

   -- Broadcast spam
   refuelBroadcast()
end


-- Handles collective death
function col_dead ()
   col_alive = col_alive - 1 -- Another one bites the dust

   -- All dead -> area clear
   if col_alive == 0 then
      if misn_stage == 1 then
         misn.setDesc( string.format(misn_desc[2], misn_final_sys:name() ))
         player.msg( string.format( talk[1], misn_final_sys:name() ))
         misn.markerMove( misn_marker, misn_final_sys )
         misn_stage = 2
      elseif misn_stage == 3 then
         misn.setDesc( string.format(misn_desc[3], misn_base:name(), misn_base_sys:name() ))
         player.msg( string.format( talk[2], misn_base_sys:name() ))
         misn.markerMove( misn_marker, misn_base_sys )
         misn_stage = 4
      end

      -- Refuel ship enters
      misn.timerStart( "addRefuelShip", 3000 )
   end
end


-- Handles arrival back to base
function land ()
   pnt = planet.get()

   -- Final landing stage
   if misn_stage == 4 and pnt == misn_base then

      tk.msg( title[3], string.format(text[3], misn_base:name()) )

      -- Rewards
      player.modFaction("Empire",5)
      diff.apply("collective_dead")
      player.pay( 500000 ) -- 500k

      tk.msg( title[3], text[4] )

      misn.finish(true) -- Run last
   end
end

