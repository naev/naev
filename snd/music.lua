
--[[
-- music will get called with a string parameter indicating status
-- valid parameters:
--    load - game is loading
--    land - player landed
--    takeoff - player took off
--    combat - player just got a hostile onscreen
--    idle - current playing music ran out
]]--
last = "idle"
function choose( str )

   if str == "load" then

      music.load( "machina" )
      music.play()

   elseif str == "land" then
  
      planet = space.landName()
      class = space.planetClass(planet)

      if class == "M" then
         mus = "agriculture"
      elseif name == "Anecu" then -- TODO we need a way to differenciate aquatics
         mus = "ocean"
      elseif class == "P" then
         mus =  "snow"
      else
         if space.planetServices(planet) > 0 then
            mus = "cosmostation"
         else
            mus = "agriculture"
         end
      end

      music.load( mus )
      music.play()

   elseif str == "takeoff" then

      takeoff = { "liftoff", "launch2", "launch3chatstart" }
      music.load( takeoff[ rnd.int(1,#takeoff) ])
      music.play()

   elseif str == "ambient" then

      ambient = { "peace1", "mission", "peace2", "peace4", "peace6" }
      music.load( ambient[ rnd.int(1,#ambient) ])
      music.play()

   elseif str == "combat" then

      music.load( "galacticbattle" )
      music.play()

   elseif str == "idle" and last ~= "idle" then

      -- We'll play the same as last unless it was takeoff
      if last == "takeoff" then
         choose("ambient")
      else
         choose(last)
      end
   end

   if str ~= "idle" then
      last = str -- save the last string so we can use it
   end
end


