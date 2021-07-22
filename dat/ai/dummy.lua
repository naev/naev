require 'ai.core.core'
--[[
-- Dummy AI - does nothing except brake and then float around.
--]]

-- Required control rate
control_rate = 2

function create ()
   ai.pushtask( "brake" )
end

-- No need for control
function control () end
function control_manual () end

-- No response
function attacked ( attacker ) end

-- Does nothing
function idle () end

-- Brakes the pilot
function brake ()
   ai.brake()
   if ai.isstopped() then
      ai.stop()
      ai.poptask()
   end
end

