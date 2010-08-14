
function background ()

   -- We can do systems without nebula
   sys = system.cur()
   nebud, nebuv = sys:nebula()
   if nebud > 0 then
      return
   end


   -- Set up parameters
   path  = ""
   x     = 0
   y     = 0
   scale = 1
   move  = 0.1
   col   = colour.new()
   --img = tex.open( path )
   --bkg.image( img, x, y, move, scale, col )

end



