include("scripts/prng.lua")

nebulae = {
   "nebula02.png",
   "nebula04.png",
   "nebula10.png",
   "nebula12.png",
   "nebula16.png",
   "nebula17.png",
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
   "nebula33.png",
   "nebula34.png",
}


stars = {
   "blue01.png",
   "blue02.png",
   "blue04.png",
   "green01.png",
   "green02.png",
   "orange01.png",
   "orange02.png",
   "orange05.png",
   "redgiant01.png",
   "white01.png",
   "white02.png",
   "yellow01.png",
   "yellow02.png"
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
   local nmove = prng.num()*0.04
   local move  = 0.02 + nmove
   local scale = 1.0 - (1. - nmove/0.2)/5
   bkg.image( img, x, y, move, scale ) -- On the background
   return num
end


