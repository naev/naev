--[[

      AI for stationary turrets.

--]]



control_rate = 2

function control ()
   local task = ai.taskname()
   local enemy = ai.getenemy()

   if task == "stationary" then
      if enemy ~= nil then
         ai.pushtask( "engage", enemy )
      end
   else
      -- Todo something about distance
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
   local range = ai.getweaprange( 3 )
   if dist < range then
      -- In melee
      local dir = ai.aim( target )
      ai.weapset( 3 )
      if dir < 10 then
         ai.shoot()
      end
      ai.shoot(true)
   else
      -- Long-range
      ai.face( target )
      ai.weapset( 4 )
      ai.shoot()
      ai.shoot(true)
   end
end


