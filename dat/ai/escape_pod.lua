-- For escape pods, only accelerates

local noop = function () end
create = noop
control_manual = noop
refuel = noop

function control ()
   if ai.taskname() then return end
   ai.pushtask( "accel" )
end

function accel ()
   ai.accel()
end
