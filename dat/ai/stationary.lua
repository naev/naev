--[[

      AI for stationary turrets.

--]]


require("dat/ai/include/basic.lua")


control_rate = 2


function create ()
   ai.pushtask( "stationary" )
end


function control ()
   local task = ai.taskname()
   local enemy = ai.getenemy()

   if task == "stationary" then
      if enemy ~= nil then
         ai.pushtask( "attack", enemy )
      end
   else
      attack_nearest( enemy )
   end
end


function attack_nearest( hostile )
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
      ai.pushtask( "attack", hostile )
   end
end


function attacked( hostile )
   local task = ai.taskname()

   if task == "stationary" then
      ai.pushtask( "attack", hostile )
   else
      attack_nearest( hostile )
   end
end


function stationary ()
   -- Do nothing
end


function attack ()
   local target = ai.target()

   -- Stop attacking if it doesn't exist
   if not target:exists() then
      ai.poptask()
      return
   end

   -- Set the target
   ai.settarget( target )

   -- Choose how to face
   local dist  = ai.dist( target )
   local range = ai.getweaprange( 3 ) -- Short range
   local dir
   if dist < range then
      dir = ai.aim( target )
   else
      dir = ai.face( target )
   end

   -- In melee
   if dist < range then
      ai.weapset( 3 ) -- Forward/turret
      if dir < 10 then
         ai.shoot() -- Forward
      end
      ai.shoot(true) -- Turret
   end

   -- Long-range
   if dir < 10 then
      ai.weapset( 4 ) -- Missiles, it's a fire group
   end
end


