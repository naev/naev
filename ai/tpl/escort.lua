include("ai/tpl/generic.lua")

-- Shouldn't think, should only obey orders.
atk_think = false
command = true


-- Simple create function
function create ( master )
   mem.escort = master
   mem.carrier = true
   attack_choose()
end

-- Just tries to guard mem.escort
function idle ()
   ai.pushtask(0, "escort")
end

-- Escorts the target
function escort ()
   target = mem.escort

   -- Will just float without a target to escort.
   if target == nil then
      return
   end
   
   dir = ai.face(target)
   dist = ai.dist( ai.pos(target) )
   bdist = ai.minbrakedist()

   -- Close enough.
   if ai.isstopped() and dist < 300 then
      return

   -- Brake
   elseif dist+100 < bdist then
      ai.pushtask(0, "brake")

   -- Must approach
   elseif dir < 10 and dist > 300 then
      ai.accel()

   end
end

-- Just brakes
function brake ()
   ai.brake()
   if ai.isstopped() then
      ai.poptask()
   end
end


-- Holds position
function hold ()
   if not ai.isstopped() then
      ai.brake()
   end
end


-- Tries to fly back to carrier
function flyback ()
   target = mem.escort

   dir = ai.face(target)
   dist = ai.dist( ai.pos(target) )
   bdist = ai.minbrakedist()

   -- Try to brake
   if not ai.isstopped() and dist < bdist then
      ai.pushtask(0, "brake")

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
   if command then
      ai.pushtask(0, "attack", target)
   end
end
-- Hold position
function e_hold ()
   if command then
      ai.pushtask(0, "hold" )
   end
end
-- Return to carrier
function e_return ()
   if command and mem.carrier then
      ai.pushtask(0, "flyback" )
   end
end
-- Clear orders
function e_clear ()
   if command then
      while ai.taskname() ~= "none" do
         ai.poptask()
      end
   end
end
