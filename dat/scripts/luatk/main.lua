local luatk = require 'luatk.core'

function love.keypressed( key )
   luatk.keypressed( key )
end

function love.mousepressed( mx, my, button )
   luatk.mousepressed( mx, my, button )
end

function love.mousereleased( mx, my, button )
   luatk.mousereleased( mx, my, button )
end

function love.mousemoved( mx, my, dx, dy )
   luatk.mousemoved( mx, my, dx, dy )
end

function love.draw()
   luatk.draw()
end

function love.update(dt)
   luatk.update(dt)
end

function love.load()
   -- Transparent background in Naev
   love.graphics.setColor( 1, 1, 1, 1 )
   love.graphics.setBackgroundColor( 0, 0, 0, 0 )
end
