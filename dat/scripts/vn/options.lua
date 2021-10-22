local luatk = require "luatk"

local opt = {}

function opt.mousepressed( mx, my, btn )
   return luatk.mousepressed( mx, my, btn )
end
function opt.mousemoved( mx, my )
   return luatk.mousemoved( mx, my )
end
function opt.keypressed( key )
   return luatk.keypressed( key )
end
function opt.draw ()
   luatk.draw()
end
function opt.update( dt )
   luatk.update( dt )
   return opt.running
end

function opt.open ()
   local w = 800
   local h = 600
   local wdw = luatk.newWindow( nil, nil, 800, 600 )

   luatk.newButton( wdw, w-120-20, h-40-20, 120, 40, _("Done"), function ()
      luatk.close()
      opt.running = false
   end )

   opt.running = true
end

return opt
