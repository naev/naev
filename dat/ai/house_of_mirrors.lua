require 'ai.core.core'
require 'ai.core.control.escort'

control_rate = math.huge

function create ()
   create_pre()
   create_post()
end

function control ()
   if not ai.taskname() then
      ai.pushtask( "faceleader" )
   end
end

function control_manual ()
end

function refuel ()
end

function idle ()
end

-- luacheck: globals faceleader (AI Task functions passed by name)
function faceleader()
   local l = ai.pilot():leader()
   if not l or not l:exists() then
      ai.poptask()
      return
   end

   ai.face( l:dir() )
end

-- luacheck: globals shootat (AI Task functions passed by name)
function shootat( target )
   local p = ai.pilot()
   if not target or not target:exists() or not p:faction():areEnemies(target:faction()) then
      ai.poptask()
      return
   end

   ai.face( target )
   ai.shoot()
end
