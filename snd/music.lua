
--[[
-- music will get called with a string parameter indicating status
-- valid parameters:
--    load - game is loading
--    land - player landed
--    takeoff - player took off
--    combat - player just got a hostile onscreen
--    idle - current playing music ran out
]]--
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

   elseif str == "combat" then

      music.load( "galacticbattle" )
      music.play()

   elseif str == "idle" then

      choose(last) -- this should be smarter in the future

   end

   last = str -- save the last string so we can use it
end


