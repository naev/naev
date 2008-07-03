
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

      music.load( "agriculture" )
      music.play()

   elseif str == "takeoff" then

      music.load( "liftoff" )
      music.play()

   elseif str == "ambient" then

      music.load( "machina" )
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


