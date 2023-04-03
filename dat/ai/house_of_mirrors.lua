require 'ai.core.core'
require 'ai.core.control.escort'

control_rate = math.huge

function create ()
   create_pre()
   create_post()
end

function control ()
end

function control_manual ()
end

function refuel ()
end

function idle ()
end

-- luacheck: globals shootat (AI Task functions passed by name)
function shootat( target )
   if not target or not target:exists() then
      ai.poptask()
      return
   end

   ai.face( target )
   ai.shoot()
end
