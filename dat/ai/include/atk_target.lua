--[[
--    Nongeneric Think functions for determining who to attack are found here
--    attack.lua contains bindings to these functions by ship class
--]]


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

   local range  = ai.getweaprange(3, 1)
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
-- big game hunter attack pattern using heuristic target identification.
--]]
function atk_heuristic_big_game_think ()
   local enemy         = ai.getenemy_heuristic(0.9, 0.9, 0.9, 20000)
   local nearest_enemy = ai.getenemy()
   local target = ai.target()
   local dist = ai.dist(target)

   -- Stop attacking if it doesn't exist
   if not target:exists() then
      ai.poptask()
      return
   end

   local range = ai.getweaprange(3, 0)
   -- Get new target if it's closer
   -- prioritize targets within the size limit
   if enemy ~= target and enemy ~= nil then
      -- Shouldn't switch targets if close
      if dist > range * mem.atk_changetarget then
         ai.pushtask("attack", enemy )
      end
      
   elseif nearest_enemy ~= target and nearest_enemy ~= nil then
      -- Shouldn't switch targets if close
      if dist > range * mem.atk_changetarget then
         ai.pushtask("attack", nearest_enemy )
      end
   end
end
