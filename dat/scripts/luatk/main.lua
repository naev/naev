local luatk = require 'luatk.core'
local le = require 'love.event'

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

function love.textinput( str )
   return luatk.textinput( str )
end

function love.draw()
   luatk.draw()
end

local function _update( dt )
   if #luatk._windows<=0 then
      le.quit()
      return
   end
   luatk.update(dt)
end

-- OK, so we actually use a pseudo update to initialize the focus and then pass
-- over to the normal focus. This has to be done, otherwise the focus gets set
-- before the C-side toolkit releases focus, which can lead to issues with
-- setTextInput. Basically, this initial update function exists to initialize
-- necessary things after window creation.
function love.update( dt )
   -- Start focus
   local wdw = luatk._windows[ #luatk._windows ]
   if not wdw then
      return
   end
   for _k,wgt in ipairs(wdw._widgets) do
      if wgt.focus then
         wgt:focus()
      end
   end

   love.update = _update
   _update( dt )
end

function love.load()
   -- Transparent background in Naev, use the true love2d API here
   love.graphics.setColor( 1, 1, 1, 1 )
   love.graphics.setBackgroundColor( 0, 0, 0, 0 )
end
