include("dat/ai/tpl/generic.lua")

-- Shouldn't think, should only obey orders.
mem.command = true


-- Simple create function
function create ( master )
   mem.escort  = master
   mem.carrier = true
   attack_choose()

   -- Disable thinking
   mem.atk_think = nil
end

-- Just tries to guard mem.escort
function idle ()
   ai.pushtask("follow", mem.escort)
end


-- Holds position
function hold ()
   if not ai.isstopped() then
      ai.brake()
   end
end


-- Tries to fly back to carrier
function flyback ()
   local target = mem.escort
   local dir    = ai.face(target)
   local dist   = ai.dist(target)
   local bdist  = ai.minbrakedist()

   -- Try to brake
   if not ai.isstopped() and dist < bdist then
      ai.pushtask("brake")

   -- Try to dock
   elseif ai.isstopped() and dist < 30 then
      ai.dock(target)

   -- Far away, must approach
   elseif dir < 10 then
      ai.accel()
   end
end


--[[
--    Escort commands
--]]
-- Attack target
function e_attack( target )
   if mem.command then
      if target ~= nil then
         ai.pushtask("attack", target)
      end
   end
end
-- Hold position
function e_hold ()
   if mem.command then
      ai.pushtask("hold" )
   end
end
-- Return to carrier
function e_return ()
   if mem.command and mem.carrier then
      ai.pushtask("flyback" )
   end
end
-- Clear orders
function e_clear ()
   if mem.command then
      while ai.taskname() ~= "none" do
         ai.poptask()
      end
   end
end
