--[[
   Simply spawns some targets and messes up with the camera to test weapons.
--]]
return function ( ship )
   if ship == nil then ship = "Mule" end
   player.teleport("Delta Pavonis")
   player.pilot():setPos( vec2.new( 0, 8000 ) )
   pilot.clear()
   pilot.toggleSpawn(false)

   local start = vec2.new( 800, 0 )
   local pos = {
      vec2.new( 000,    0 ),
      vec2.new( 200,    0 ),
      --vec2.new( 400,    0 ),
      vec2.new( 100,  100 ),
      vec2.new( 100, -100 ),
      vec2.new( 300,  100 ),
      vec2.new( 300, -100 ),
      vec2.new( 300,  300 ),
      vec2.new( 300, -300 ),
      vec2.new( 200,  200 ),
      vec2.new( 200, -200 ),
   }
   local plts = {}
   for k,p in ipairs(pos) do
      local plt = pilot.add( ship, "Pirate", player.pos()+start+p )
      plt:control()
      plt:setNoDeath(true)
      plt:setHostile(true)
      plt:face( player.pilot() )
      plt:brake()
      table.insert( plts, plt )
   end
   camera.set( player.pos()+start )
end
