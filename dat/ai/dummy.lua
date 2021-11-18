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
function attacked ( _attacker ) end

-- Does nothing
-- luacheck: globals idle (AI Task functions passed by name)
function idle () end

-- Brakes the pilot
function brake ()
   ai.brake()
   if ai.isstopped() then
      ai.stop()
      ai.poptask()
   end
end

