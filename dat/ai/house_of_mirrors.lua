function create ()
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
