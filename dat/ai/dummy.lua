--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--]]

include("dat/ai/include/basic.lua")
--[[
-- Dummy AI - does nothing except brake and then float around.
--]]

-- Required control rate
control_rate = 2

function create ()
   ai.pushtask( "idle" )
   ai.pushtask( "brake" )
end

-- No need for control
function control ()
end

-- No response
function attacked ( attacker )
end

-- Does nothing
function idle ()
end

-- Brakes the pilot
function brake ()
   ai.brake()
   if ai.isstopped() then
      ai.stop()
      ai.poptask()
   end
end

