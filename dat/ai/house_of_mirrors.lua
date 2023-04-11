require 'ai.core.core'
require 'ai.core.control.escort'

control_rate = math.huge -- don't actually run control

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
   --local p = ai.pilot()
   --if not target or not target:exists() or not p:faction():areEnemies(target:faction()) then
   if not target or not target:exists() then -- The enemy check is handled in the outfit function
      ai.poptask()
      return
   end
   ai.weapset( 3 ) -- Turret and forward weapons
   ai.settarget( target )

   local dir = ai.aim( target )
   if dir < math.rad(10) then
      ai.shoot()
   end
   ai.shoot(true)
end
