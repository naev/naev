--[[

      AI for stationary turrets.

--]]



control_rate = 2


function create ()
   ai.pushtask( "stationary" )
end


function control ()
   local task = ai.taskname()
   local enemy = ai.getenemy()

   if task == "stationary" then
      if enemy ~= nil then
         ai.pushtask( "engage", enemy )
      end
   else
      engage_nearest( enemy )
   end
end


function engage_nearest( hostile )
   -- Must not be same
   local target       = ai.target()
   if target == hostile then
      return
   end

   -- Engage closest
   local dist_hostile = ai.dist( hostile )
   local dist_enemy   = ai.dist( target )
   if dist_hostile < dist_enemy*0.75 then
      ai.poptask()
      ai.pushtask( "engage", hostile )
   end
end


function attacked( hostile )
   local task = ai.taskname()

   if task == "stationary" then
      ai.pushtask( "engage", hostile )
   else
      engage_nearest( hostile )
   end
end


function stationary ()
   -- Do nothing
end


function engage ()
   local target = ai.target()

   -- Stop attacking if it doesn't exist
   if not ai.exists(target) then
      ai.poptask()
      return
   end

   -- Check melee range
   local dist  = ai.dist( target )
   local range = ai.getweaprange( 3 ) -- Short range
   if dist < range then
      -- In melee
      local dir = ai.aim( target )
      ai.weapset( 3 ) -- Forward/turret
      if dir < 10 then
         ai.shoot() -- Forward
      end
      ai.shoot(true) -- Turret
   else
      -- Long-range
      ai.face( target )
      ai.weapset( 4 ) -- Missiles, it's a fire group
   end
end


