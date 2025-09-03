
-- Applies a function to all ships
return function ( func )
   local curship = player.pilot():name()
   local ships = player.ships()
   local deployed = {}
   for k,s in ipairs( ships ) do
      if s.deployed then
         table.insert( deployed, s )
      end
   end
   for k,s in ipairs( ships ) do
      player.shipSwap( s.name, true )
      func( player.pilot() )
   end
   player.shipSwap( curship, true )
   func( player.pilot() )
   for k,s in ipairs( deployed ) do
      player.shipDeploy( s.name, true )
   end
end
