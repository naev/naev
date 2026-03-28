local vn = require 'vn.vn'

-- luacheck: globals love (vn overrides parts of love2d.)

function love.keypressed( key, _scancode, isrepeat )
   return vn.keypressed( key, isrepeat )
end

function love.mousepressed( mx, my, button )
   return vn.mousepressed( mx, my, button )
end

function love.mousereleased( mx, my, button )
   return vn.mousereleased( mx, my, button )
end

function love.mousemoved( mx, my, dx, dy )
   return vn.mousemoved( mx, my, dx, dy )
end

function love.wheelmoved( dx, dy )
   return vn.wheelmoved( dx, dy )
end

function love.textinput( str )
   return vn.textinput( str )
end

function love.draw()
   vn.draw()
end

function love.update(dt)
   vn.update(dt)
end

function love.resize( w, h )
   vn.resize( w, h )
end

function love.load()
   -- Transparent background in Naev
   love.graphics.setColour( 1, 1, 1, 1 )
   love.graphics.setBackgroundColour( 0, 0, 0, 0 )

   -- Check to see if running standalone
   if not love._vn then
      error("VN is not set up!")
   end

   -- Insert start and end state if necessary
   table.insert( vn._states, 1, vn.StateStart.new() )
   --table.insert( vn._states, vn.StateEnd.new() )
   vn.done()
   vn._started = true
   vn._state = 1
   vn._fade = 0
   local s = vn._states[ vn._state ]
   s:init()
   vn._checkDone()
end
