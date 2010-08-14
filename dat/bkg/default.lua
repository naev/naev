include("scripts/prng.lua")

function background ()

   -- We can do systems without nebula
   local sys = system.cur()
   local nebud, nebuv = sys:nebula()
   if nebud > 0 then
      return
   end

   local nebulae = {
      "nebula01.png",
      "nebula02.png",
      "nebula03.png",
      "nebula04.png",
      "nebula05.png",
      "nebula06.png",
      "nebula07.png",
      "nebula08.png",
      "nebula09.png",
      "nebula10.png",
      "nebula11.png",
      "nebula12.png",
      "nebula13.png",
      "nebula14.png",
      "nebula15.png",
      "nebula16.png",
      "nebula17.png",
      "nebula18.png",
      "nebula19.png",
      "nebula20.png",
      "nebula21.png",
      "nebula22.png",
      "nebula23.png",
      "nebula24.png",
      "nebula25.png",
      "nebula26.png",
      "nebula27.png",
      "nebula28.png",
      "nebula29.png",
      "nebula30.png",
      "nebula31.png",
      "nebula32.png",
      "nebula33.png"
   }

   prng.initHash( system.name(sys) )

   -- Set up parameters
   local path  = "gfx/bkg/"
   local nebula = nebulae[ prng.range(1,#nebulae) ]
   local img   = tex.open( path .. nebula )
   local w,h   = img:dim()
   local x     = (prng.num( prng.z ) - .5) * 10000
   local y     = (prng.num( prng.z ) - .5) * 10000
   local move  = 0.1 + prng.num()*0.4
   local scale = 1 + (prng.num()*0.5 + 0.5)*((2000+2000)/(w+h))
   bkg.image( img, x, y, move, scale )

end



