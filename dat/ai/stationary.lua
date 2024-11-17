--[[
      AI for stationary turrets.
--]]
require 'ai.core.core'
local atk = require "ai.core.attack.util"

control_rate = 2

local function attack_nearest( hostile )
   -- Must not be same
   local target       = ai.taskdata()
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


function create ()
   ai.pushtask( "stationary" )
end


function control( dt )
   mem.elapsed = mem.elapsed + dt
   local task = ai.taskname()
   local enemy = atk.preferred_enemy()

   if task == "stationary" then
      if enemy ~= nil then
         ai.pushtask( "attack", enemy )
      end
   else
      attack_nearest( enemy )
   end
end


control_manual = control


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
   local target = ai.taskdata()

   -- Stop attacking if it doesn't exist
   if not target:exists() then
      ai.poptask()
      return
   end

   -- Set the target
   ai.settarget( target )

   -- Choose how to face
   local dist  = ai.dist( target )
   local range = atk.primary_range()
   if dist < range then
      ai.aim( target )
   else
      ai.face( target )
   end

   -- Fire all weapons, let inrange decideIn melee
   atk.primary()
   atk.secondary()
end
