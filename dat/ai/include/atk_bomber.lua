function atk_bomber_init ()
   mem.atk_think  = atk_heuristic_big_game_think
   mem.atk        = atk_bomber
end


--[[
-- Main control function for bomber behavior.
-- Bombers are expected to have heavy weapons and target
--ships bigger than they are
--]]
function atk_bomber ()
   local target = _atk_com_think()
   if target == nil then return end

   -- Targeting stuff
   ai.hostile(target) -- Mark as hostile
   ai.settarget(target)

   -- See if the enemy is still seeable
   if not _atk_check_seeable() then return end

   -- Get stats about enemy
   local dist  = ai.dist( target ) -- get distance
   local range = ai.getweaprange(3, 0)

   -- TODO bombers need their own specific routines
   if dist > range * mem.atk_approach then
      _atk_g_ranged( target, dist )

   else
      _atk_f_flyby( target, dist )   
   end
end

