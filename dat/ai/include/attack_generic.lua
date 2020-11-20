--[[
--    Generic attack functions
--]]

--[[
-- Mainly manages targeting nearest enemy.
--]]
function atk_g_think ()
   local enemy = ai.getenemy()
   local target = ai.target()

   -- Stop attacking if it doesn't exist
        if not target:exists() then
                ai.poptask()
                return
        end

   -- Get new target if it's closer
   if enemy ~= target and enemy ~= nil then
      local dist = ai.dist( target )
      local range = ai.getweaprange( 0 )

      -- Shouldn't switch targets if close
      if dist > range * mem.atk_changetarget then
         ai.pushtask( "attack", enemy )
      end
   end
end


--[[
-- Attacked function.
--]]
function atk_g_attacked( attacker )
   local target = ai.target()

   if mem.recharge then
      mem.recharge = false
   end

   -- If no target automatically choose it
   if not target:exists() then
      ai.pushtask("attack", attacker)
      return
   end

   local tdist  = ai.dist(target)
   local dist   = ai.dist(attacker)
   local range  = ai.getweaprange( 0 )

   if target ~= attacker and dist < tdist and
         dist < range * mem.atk_changetarget then
      ai.pushtask("attack", attacker)
   end
end


--[[
-- Generic "brute force" attack.  Doesn't really do anything interesting.
--]]
function atk_g ()
        local target = ai.target()

        -- make sure pilot exists
        if not target:exists() then
                ai.poptask()
                return
        end

   -- Check if is bribed by target
   if ai.isbribed(target) then
      ai.poptask()
      return
   end

   -- Check if we want to board
   if mem.atk_board and ai.canboard(target) then
      ai.pushtask( "board", target );
      return
   end

   -- Check to see if target is disabled
   if not mem.atk_kill and target:flags().disabled then
      ai.poptask()
      return
   end

   -- Targeting stuff
   ai.hostile(target) -- Mark as hostile
   ai.settarget(target)

   -- Get stats about enemy
        local dist  = ai.dist( target ) -- get distance
   local range = ai.getweaprange( 0 )

   -- We first bias towards range
   if dist > range * mem.atk_approach then
      atk_g_ranged( target, dist )

   -- Now we do an approach
   elseif dist > range * mem.atk_aim then
      atk_g_approach( target, dist )

   -- Close enough to melee
   else
      atk_g_melee( target, dist )
   end
end


--[[
-- Enters ranged combat with the target
--]]
function atk_g_ranged( target, dist )
   local dir = ai.face(target) -- Normal face the target

   -- Check if in range
   if dist < ai.getweaprange( 4 ) and dir < 30 then
      ai.weapset( 4 )
   end

   -- Always launch fighters
   ai.weapset( 5 )

   -- Approach for melee
   if dir < 10 then
      ai.accel()
   end
end


--[[
-- Approaches the target
--]]
function atk_g_approach( target, dist )
   dir = ai.idir(target)
   if dir < 10 and dir > -10 then
      _atk_keep_distance()
   else
      dir = ai.iface(target)
   end
   
   if dir < 10 then
      ai.accel()
   end
end


--[[
-- Melees the target
--]]
function atk_g_melee( target, dist )
   local dir = ai.aim(target) -- We aim instead of face
   local range = ai.getweaprange( 3 )

   -- Set weapon set
   ai.weapset( 3 )

   -- Drifting away we'll want to get closer
   if dir < 10  and dist > 0.5*range and ai.relvel(target) > -10 then
      ai.accel()
   end

   -- Shoot if should be shooting.
   if dir < 10 then
      local range = ai.getweaprange( 3, 0 )
      if dist < range then
         ai.shoot()
      end
   end
   if ai.hasturrets() then
      local range  = ai.getweaprange( 3, 1 )
      if dist < range then
         ai.shoot(true)
      end
   end
end


--[[
-- Approaches the target evasively, never heading in a straight line
-- This will tend to approach a target along a loose spiral, good for evading capship guns
--]]
function atk_spiral_approach( target, dist )
   local dir  = ai.idir(target)
   local adir = math.abs(dir)

   --these two detect in-cone approach vectors
   if adir > 10 and adir < 30 then
      ai.accel()
   end

   --facing away from the target, turn to face
   if adir > 30 then
      ai.iface(target)
   end

   --aiming right at the target; turn away
   if dir > 0 and dir < 10 then
      ai.turn(1)
   elseif dir < 0 and dir > -10 then
      ai.turn(-1)
   end
end -- end spiral approach


--[[
--Attempts to maintain a constant distance from nearby things
--This modulates the distance between the current pilot and its nearest neighbor
--]]
function keep_distance()

   --anticipate this will be added to eliminate potentially silly behavior if it becomes a problem
   --local flight_offset = ai.drift_facing()

   --find nearest thing
   local neighbor = ai.nearestpilot()
   if not neighbor or not neighbor:exists() then
      return
   end

   --find the distance based on the direction I'm travelling
   local perp_distance = ai.flyby_dist(neighbor)
   -- adjust my direction of flight to account for this
   -- if pilot is too close, turn away
   if perp_distance < 0 and perp_distance > -50 then
      ai.turn(1)
   elseif perp_distance > 0 and perp_distance < 50 then
      ai.turn(-1)
   end    
end -- end keep distance


--[[
-- Mainly targets small fighters.
--]]
function atk_fighter_think ()

   local enemy = ai.getenemy_size(0, 200)
   local nearest_enemy = ai.getenemy()
   local dist = 0
   local sizedist = 0

   if enemy ~= nil then
      sizedist = ai.dist(enemy)
   end   
   
   if  nearest_enemy ~= nil then   
      dist = ai.dist(nearest_enemy)
   end

   local target = ai.target()

   -- Stop attacking if it doesn't exist
   if not target:exists() then
      ai.poptask()
      return
   end

   local range = ai.getweaprange(3, 0)

   -- Get new target if it's closer
   --prioritize targets within the size limit
   if enemy ~= target and enemy ~= nil then
      -- Shouldn't switch targets if close
      if sizedist > range * mem.atk_changetarget then
         ai.pushtask("attack", enemy )
      end
      
   elseif nearest_enemy ~= target and nearest_enemy ~= nil then

      -- Shouldn't switch targets if close
      if dist > range * mem.atk_changetarget then
         ai.pushtask("attack", nearest_enemy )
      end
   end
end


--[[
-- Mainly targets in biggest-to-smallest priority.
--]]
function atk_topdown_think ()

   local enemy_cat1 = ai.getenemy_size(2500, 10000)
   local enemy_cat2 = ai.getenemy_size(1000, 2500)
   local enemy_cat3 = ai.getenemy_size(600, 1000)
   local enemy_cat4 = ai.getenemy_size(250, 600)
   
   
   local nearest_enemy = ai.getenemy()
   local dist = 0
   local cat1dist = 0
   local cat2dist = 0
   local cat3dist = 0
   local cat4dist = 0

   if enemy_cat1 ~= nil then
      cat1dist = ai.dist(enemy_cat1)
   end   

   if enemy_cat2 ~= nil then
      cat2dist = ai.dist(enemy_cat2)
   end

   if enemy_cat3 ~= nil then
      cat3dist = ai.dist(enemy_cat3)
   end   
   
   if enemy_cat4 ~= nil then
      cat4dist = ai.dist(enemy_cat4)
   end
   
   if  nearest_enemy ~= nil then   
      dist = ai.dist(nearest_enemy)
   end

   local target = ai.target()

   -- Stop attacking if it doesn't exist
        if not target:exists() then
                ai.poptask()
                return
        end

   local range = ai.getweaprange(3, 1)
   local range2 = ai.getweaprange(3, 0)
   
   if range2 > range then
    range = range2
   end

   -- Get new target if it's closer
   if enemy_cat1 ~= target and enemy_cat1 ~= nil then  

      -- Shouldn't switch targets if close
      if cat1dist > range * mem.atk_changetarget then
         ai.pushtask("attack", enemy_cat1 )
      end
   
   elseif enemy_cat2 ~= target and enemy_cat2 ~= nil then  

      -- Shouldn't switch targets if close
      if cat2dist > range * mem.atk_changetarget then
         ai.pushtask("attack", enemy_cat2 )
      end

   elseif enemy_cat3 ~= target and enemy_cat3 ~= nil then  

      -- Shouldn't switch targets if close
      if cat3dist > range * mem.atk_changetarget then
         ai.pushtask("attack", enemy_cat3 )
      end
      
   elseif enemy_cat4 ~= target and enemy_cat4 ~= nil then  

      -- Shouldn't switch targets if close
      if cat4dist > range * mem.atk_changetarget then
         ai.pushtask("attack", enemy_cat4 )
      end   
      
   elseif nearest_enemy ~= target and nearest_enemy ~= nil then


      -- Shouldn't switch targets if close
      if dist > range * mem.atk_changetarget then
         ai.pushtask("attack", nearest_enemy )
      end
   end
end


--[[
-- Main control function for fighter behavior.
--]]
function atk_fighter ()
    
   local target = ai.target()

   -- make sure pilot exists
   if not target:exists() then
           ai.poptask()
           return
   end

   -- Check if is bribed by target
   if ai.isbribed(target) then
      ai.poptask()
      return
   end

   -- Check if we want to board
   if mem.atk_board and ai.canboard(target) then
      ai.pushtask("board", target );
      return
   end

   -- Check to see if target is disabled
   if not mem.atk_kill and target:flags().disabled then
      ai.poptask()
      return
   end

   -- Targeting stuff
   ai.hostile(target) -- Mark as hostile
   ai.settarget(target)

   -- Get stats about enemy
   local dist  = ai.dist( target ) -- get distance
   local range = ai.getweaprange(3, 0)

   -- We first bias towards range
   if dist > range * mem.atk_approach then
      atk_g_ranged( target, dist )

   -- engage melee
   else
      if target:stats().mass < 200 then
        atk_g_space_sup(target, dist)
      else
        atk_g_flyby_aggressive( target, dist )
      end
   end
end

--[[
-- Main control function for corvette behavior.
--]]
function atk_corvette ()

   local target = ai.target()

   -- make sure pilot exists
   if not target:exists() then
           ai.poptask()
           return
   end

   -- Check if is bribed by target
   if ai.isbribed(target) then
      ai.poptask()
      return
   end

   -- Check if we want to board
   if mem.atk_board and ai.canboard(target) then
      ai.pushtask("board", target );
      return
   end

   -- Check to see if target is disabled
   if not mem.atk_kill and target:flags().disabled then
      ai.poptask()
      return
   end

   -- Targeting stuff
   ai.hostile(target) -- Mark as hostile
   ai.settarget(target)

   -- Get stats about enemy
   local dist  = ai.dist( target ) -- get distance
   local range = ai.getweaprange(3, 0)
   local range2 = ai.getweaprange(3, 1)
   
   if range2 > range then
    range = range2
   end

   -- We first bias towards range
   if dist > range * mem.atk_approach then
      atk_g_ranged( target, dist )


   -- Close enough to melee
   else
      if target:stats().mass < 500 then
        atk_g_space_sup(target, dist)
      else
        atk_g_flyby_aggressive( target, dist )
      end
   end
end


--[[
-- Main control function for capital ship behavior.
--]]
function atk_capital ()

   local target = ai.target()

   -- make sure pilot exists
   if not target:exists() then
           ai.poptask()
           return
   end

   -- Check if is bribed by target
   if ai.isbribed(target) then
      ai.poptask()
      return
   end

   -- Check if we want to board
   if mem.atk_board and ai.canboard(target) then
      ai.pushtask("board", target );
      return
   end

   -- Check to see if target is disabled
   if not mem.atk_kill and target:flags().disabled then
      ai.poptask()
      return
   end

   -- Targeting stuff
   ai.hostile(target) -- Mark as hostile
   ai.settarget(target)

   -- Get stats about enemy
        local dist  = ai.dist( target ) -- get distance
   local range = ai.getweaprange(3, 1)

   -- We first bias towards range
   if dist > range * mem.atk_approach then
      atk_g_ranged( target, dist )

   -- Now we do an approach
   --elseif dist > 10 * range * mem.atk_aim then
   --   atk_spiral_approach( target, dist )

   -- Close enough to melee
   else   
     atk_g_capital(target, dist)
   end
end --end capship attack


--[[ Detail melee attack profiles follow here
--These profiles are intended for use by specific ship classes
--]]

--[[
-- Execute a sequence of close-in flyby attacks
-- Uses a combination of facing and distance to determine what action to take
--This version is slightly more aggressive and follows the target
--]]
function atk_g_flyby_aggressive( target, dist )

   --ai.pilot():comm(1, "flyby attack!")
   local range = ai.getweaprange(3)

   -- Set weapon set
   ai.weapset( 3 )

   local dir = 0
   
   --if we're far away from the target, then turn and approach 
   if dist > (3 * range) then
    
     dir = ai.idir(target)
   
     if dir < 10 and dir > -10 then
         keep_distance()     
         
     else  
         dir = ai.iface(target)
     end

      if dir < 10 and dir > -10 then
        ai.accel()
      end
      
   elseif dist > (0.75 * range) then
   
       dir = ai.idir(target)
       
       --test if we're facing the target. If we are, keep approaching
       if(dir < 30 and dir > -30) then
          ai.iface(target)
          
          if dir < 10 and dir > -10 then 
            ai.accel()
          end
         
       elseif dir >= 30 and dir <= 180 then
          ai.turn(1)
          ai.accel()
          
       else
          ai.turn(-1)
          ai.accel()
          
       end
   
   else

    --otherwise we're close to the target and should attack until we start to zip away

    dir = ai.aim(target)
    ai.accel()
    
   -- Shoot if should be shooting.
   if dir < 10 then
      range = ai.getweaprange( 3, 0 )
      if dist < range then
         ai.shoot()
      end
   end
   if ai.hasturrets() then
      range  = ai.getweaprange( 3, 1 )
      if dist < range then
         ai.shoot(true)
      end
   end

--end main if decision  
  end

--end flyby attack
end



--[[
-- Execute a sequence of close-in flyby attacks
-- Uses a combination of facing and distance to determine what action to take
--This version is slightly less aggressive and cruises by the target
--]]
function atk_g_flyby( target, dist )

   --ai.pilot():comm(1, "flyby attack")
   
   local range = ai.getweaprange(3)
   local dir = 0
   
   ai.weapset( 3 )
   
   --if we're far away from the target, then turn and approach 
   if dist > (3 * range) then
    
     dir = ai.idir(target)
   
     if dir < 10 and dir > -10 then
         keep_distance()     
         
     else  
         dir = ai.iface(target)
     end

      if dir < 10 and dir > -10 then
        ai.accel()
      end
      
   elseif dist > (0.75 * range) then
   
       dir = ai.idir(target)
       
       --test if we're facing the target. If we are, keep approaching
       if(dir <= 30 and dir > -30) then
          ai.iface(target)
          
          if dir < 10 and dir > -10 then
            ai.accel()
          end
          
       elseif dir > 30 and dir < 180 then
          ai.turn(1)
          ai.accel()
          
       else
          ai.turn(-1)
          ai.accel()
          
       end
   
   else

    --otherwise we're close to the target and should attack until we start to zip away

    dir = ai.aim(target)
    
    --not accelerating here is the only difference between the aggression levels. This can probably be an aggression AI parameter
    --ai.accel()
    
    -- Shoot if should be shooting.
    if dir < 10 then
       range = ai.getweaprange( 3, 0 )
       if dist < range then
          ai.shoot()
       end
    end
    if ai.hasturrets() then
        range  = ai.getweaprange( 3, 1 )
       if dist < range then
          ai.shoot(true)
       end
    end

--end main if decision  
  end

--end flyby attack
end


--[[
-- Simplest of all attacks: maintain an intercept course to the target, and shoot when within range
--
--This is designed for capital ships with turrets and guided munitions
--As there is no aiming involved this is a turret/capital ship only attack method
--]]
function atk_g_capital( target, dist )

   local range = ai.getweaprange(3)
   local dir = 0
   
   --ai.pilot():comm(1, "capship attack")
   
   ai.weapset( 3 )
   
   --if we're far from the target, then turn and approach 
   if dist > (range) then
   
     dir = ai.idir(target)
   
     if dir < 10 and dir > -10 then
         keep_distance()     
      
       if dir < 10 and dir > -10 then
        ai.accel()
      end
         
     else  
       dir = ai.iface(target)
       
       if dir < 10 and dir > -10 then
        ai.accel()
       end    
         
     end
   
   --at moderate range from the target, prepare to intercept and engage with turrets
   elseif dist > 0.6* range then
    
       --drifting away from target, so emphasize intercept 
       --course facing and accelerate to close
    
       dir = ai.iface(target)
    
       if dir < 10 and dir > -10 and ai.relvel(target) > -10 then 
          ai.accel()
       end
    
       if ai.hasturrets() then
          ai.shoot(true)
       end
   
   elseif dist > 0.3*range then
       
       --capital ship turning is slow
       --emphasize facing for being able to close quickly
       dir = ai.iface(target)
    
       -- Shoot if should be shooting.
       if ai.hasturrets() then
            range  = ai.getweaprange( 3, 1 )
         if dist < range then
            ai.shoot(true)
        end
       end
   
    else
    --within close range; aim and blast away with everything
    
       dir = ai.aim(target)
             
       -- Shoot if should be shooting.
       if dir < 10 then
          range = ai.getweaprange( 3, 0 )
          if dist < range then
           ai.shoot()
        end
      end
      if ai.hasturrets() then
         range  = ai.getweaprange( 3, 1 )
        if dist < range then
           ai.shoot(true)
        end
     end
    
   --end main decision if
   end


--end capital ship attack
end


--[[
-- Attack Profile for a maneuverable ship engaging a maneuverable target
--
--This is designed for fighters engaging other fighters
--
--]]
function atk_g_space_sup( target, dist )

  -- ai.pilot():comm(1, "space superiority")

   local range = ai.getweaprange(3)
   local dir = 0

   ai.weapset( 3 )
   
   --if we're far away from the target, then turn and approach 
   if dist > (range) then
   
     dir = ai.idir(target)
   
     if dir < 10 and dir > -10 then
         keep_distance()     
      
       if dir < 10 and dir > -10 then
        ai.accel()
      end
         
     else  
       dir = ai.iface(target)
       
       if dir < 10 and dir > -10 then
        ai.accel()
       end    
         
     end
     
     
   
   elseif dist > 0.8* range then
    
       --drifting away from target, so emphasize intercept 
       --course facing and accelerate to close
    
       dir = ai.iface(target)
    
       if dir < 10 and dir > -10 and ai.relvel(target) > -10 then
          ai.accel()
       end
    
       if ai.hasturrets() then
          ai.shoot(false, 1)
       end
   
   elseif dist > 0.4*range then
       
    --within close range; aim and blast away with everything
    
       dir = ai.aim(target)
       local dir2 = ai.idir(target)

       --accelerate and try to close
       --but only accel if it will be productive
       if dir2 < 15 and dir2 > -15 and ai.relvel(target) > -10 then
          ai.accel()
       end         
             
       -- Shoot if should be shooting.
       if dir < 10 then
          range = ai.getweaprange( 3, 0 )
          if dist < range then
           ai.shoot()
        end
      end
      if ai.hasturrets() then
         range  = ai.getweaprange( 3, 1 )
        if dist < range then
           ai.shoot(true)
        end
     end
   
    else
    --within close range; aim and blast away with everything
    
       dir = ai.aim(target)
             
       -- Shoot if should be shooting.
       if dir < 10 then
          range = ai.getweaprange( 3, 0 )
          if dist < range then
           ai.shoot()
        end
      end
      if ai.hasturrets() then
         range  = ai.getweaprange( 3, 1 )
        if dist < range then
           ai.shoot(true)
        end
     end
    
   --end main decision if
   end


--end space superiority ship attack
end

