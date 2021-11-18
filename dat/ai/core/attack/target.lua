--[[
--    Nongeneric Think functions for determining who to attack are found here
--    attack contains bindings to these functions by ship class
--]]


--[[
-- big game hunter attack pattern using heuristic target identification.
--]]
function atk_heuristic_big_game_think( target, _si )
   local enemy         = ai.getenemy_heuristic(0.9, 0.9, 0.9, 20000)
   local nearest_enemy = ai.getenemy()

   local dist = ai.dist(target)
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
