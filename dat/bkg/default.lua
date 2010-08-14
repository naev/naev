
function background ()

   -- We can do systems without nebula
   sys = system.cur()
   nebud, nebuv = sys:nebula()
   if nebud > 0 then
      return
   end

   --img = tex.open( "gfx/gui/minimal.png" )
   --bkg.image( img, 0, 0, 0.1, 1 )

end



