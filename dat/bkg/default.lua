prng = require "prng"

nebulae = {
   "nebula02.webp",
   "nebula04.webp",
   "nebula10.webp",
   "nebula12.webp",
   "nebula16.webp",
   "nebula17.webp",
   "nebula19.webp",
   "nebula20.webp",
   "nebula21.webp",
   "nebula22.webp",
   "nebula23.webp",
   "nebula24.webp",
   "nebula25.webp",
   "nebula26.webp",
   "nebula27.webp",
   "nebula28.webp",
   "nebula29.webp",
   "nebula30.webp",
   "nebula31.webp",
   "nebula32.webp",
   "nebula33.webp",
   "nebula34.webp",
}


stars = {
   "blue01.webp",
   "blue02.webp",
   "blue04.webp",
   "green01.webp",
   "green02.webp",
   "orange01.webp",
   "orange02.webp",
   "orange05.webp",
   "redgiant01.webp",
   "white01.webp",
   "white02.webp",
   "yellow01.webp",
   "yellow02.webp"
}


function background ()

   -- We can do systems without nebula
   cur_sys = system.cur()
   local nebud, nebuv = cur_sys:nebula()
   if nebud > 0 then
      return
   end

   -- Start up PRNG based on system name for deterministic nebula
   prng.initHash( cur_sys:name() )

   -- Generate nebula
   background_nebula()

   -- Generate stars
   background_stars()
end


function background_nebula ()
   -- Set up parameters
   local path  = "gfx/bkg/"
   local nebula = nebulae[ prng.range(1,#nebulae) ]
   local img   = tex.open( path .. nebula )
   local w,h   = img:dim()
   local r     = prng.num() * cur_sys:radius()/2
   local a     = 2*math.pi*prng.num()
   local x     = r*math.cos(a)
   local y     = r*math.sin(a)
   local move  = 0.01 + prng.num()*0.01
   local scale = 1 + (prng.num()*0.5 + 0.5)*((2000+2000)/(w+h))
   if scale > 1.9 then scale = 1.9 end
   bkg.image( img, x, y, move, scale )
end


function background_stars ()
   -- Chose number to generate
   local n
   local r = prng.num()
   if r > 0.97 then
      n = 3
   elseif r > 0.94 then
      n = 2
   elseif r > 0.1 then
      n = 1
   end

   -- If there is an inhabited planet we'll need at least one star
   if not n then
      for k,v in ipairs( cur_sys:planets() ) do
         if v:services().land then
            n = 1
            break
         end
      end
   end

   -- Generate the stars
   local i = 0
   local added = {}
   while n and i < n do
      num = star_add( added, i )
      added[ num ] = true
      i = i + 1
   end
end


function star_add( added, num_added )
   -- Set up parameters
   local path  = "gfx/bkg/star/"
   -- Avoid repeating stars
   local num   = prng.range(1,#stars)
   local i     = 0
   while added[num] and i < 10 do
      num = prng.range(1,#stars)
      i   = i + 1
   end
   local star  = stars[ num ]
   -- Load and set stuff
   local img   = tex.open( path .. star )
   local w,h   = img:dim()
   -- Position should depend on whether there's more than a star in the system
   local r     = prng.num() * cur_sys:radius()/3
   if num_added > 0 then
      r        = r + cur_sys:radius()*2/3
   end
   local a     = 2*math.pi*prng.num()
   local x     = r*math.cos(a)
   local y     = r*math.sin(a)
   local nmove = math.max( .05, prng.num()*0.1 )
   local move  = 0.02 + nmove
   local scale = 1.0 - (1. - nmove/0.2)/5
   bkg.image( img, x, y, move, scale ) -- On the background
   return num
end
