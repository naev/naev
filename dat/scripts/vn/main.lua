local utf8 = require 'utf8'
local vn = require 'vn'

function love.keypressed( key )
   vn.keypressed( key )
end

function love.mousepressed( mx, my, button )
   vn.mousepressed( mx, my, button )
end

function love.draw()
   vn.draw()
end

function love.update(dt)
   vn.update(dt)
end

function love.load()
   -- Transparent background in Naev
   love.graphics.setColor( 1, 1, 1, 1 )
   love.graphics.setBackgroundColor( 0, 0, 0, 0 )

   -- Check to see if running standalone
   if not love._vn then
      -- Small test
      vn.scene()
      --local d = vn.newCharacter( "Developer", { color={1,0,0}, image="assets/scientist.png" } )
      local d = vn.newCharacter( "Developer", { color={1,0,0}, image="gfx/portraits/neutral/scientist.png" } )
      local me = vn.me
      local na = vn.na
      na( "You reach an empty space, it almost feels like an implemented visual novel game!" )
      d( '"What are you doing here? This is still under development!"' )
      vn.menu( {
         { '"Sorry, I got lost while browsing the code."', "lost" },
         { '"I was told there would be cake."', "cake" }
      } )
      vn.label( "lost" )
      d( '"Yeah sure, you probably came looking for cake..."' )
      vn.jump( "lostcake" )
      vn.label( "cake" )
      d( '"Wait, who told you that? Nobody else was supposed to know!"' )
      vn.label( "lostcake" )
      me( '"About the cake..."' )
      na( "You see the developer give you a troublesome look..." )

      -- Set up
      love._vn = true
   end

   -- Insert start and end state if necessary
   table.insert( vn._states, 1, vn.StateStart.new() )
   table.insert( vn._states, vn.StateEnd.new() )
   vn._started = true
   vn._state = 1
   local s = vn._states[ vn._state ]
   s:init()
end

