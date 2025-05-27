-- For escape pods, only accelerates

function create ()
   ai.pushtask( "accel" )
end

local noop = function () end
control = noop
control_manual = noop
refuel = noop

function accel ()
   ai.accel()
end
