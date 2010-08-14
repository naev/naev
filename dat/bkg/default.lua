
function background ()

   -- We can do systems without nebula
   local sys = system.cur()
   local nebud, nebuv = sys:nebula()
   if nebud > 0 then
      return
   end


   -- Set up parameters
   local path  = ""
   local x     = 0
   local y     = 0
   local scale = 1
   local move  = 0.1
   local col   = colour.new()
   --local img = tex.open( path )
   --bkg.image( img, x, y, move, scale, col )

end



