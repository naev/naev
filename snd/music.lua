
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

   -- Means to only change song if needed
   if str == nil then
      str = "ambient"
   end

   if str == "load" then

      choose_load()

   elseif str == "land" then

      choose_land()

   elseif str == "takeoff" then

      choose_takeoff()

   elseif str == "ambient" then

      choose_ambient()

   elseif str == "combat" then

      choose_battle()

   elseif str == "idle" and last ~= "idle" then

      -- We'll play the same as last unless it was takeoff
      if last == "takeoff" then
         choose_ambient()
      else
         choose(last)
      end
   end

   if str ~= "idle" then
      last = str -- save the last string so we can use it
   end
end


-- Loading songs.
function choose_load ()
   music.load( "machina" )
   music.play()
end


-- Landing songs
function choose_land ()
   planet = space.getPlanet()
   class = planet:class()

   if class == "M" then
      mus = "agriculture"
   elseif name == "Anecu" then -- TODO we need a way to differenciate aquatics
      mus = "ocean"
   elseif class == "P" then
      mus = "snow"
   else
      if planet:services() > 0 then
         mus = "cosmostation"
      else
         mus = "agriculture"
      end
   end

   music.load( mus )
   music.play()
end


-- Takeoff songs
function choose_takeoff ()
   -- No need to restart
   if last == "takeoff" and music.isPlaying() then
      return
   end
   takeoff = { "liftoff", "launch2", "launch3chatstart" }
   music.load( takeoff[ rnd.int(1,#takeoff) ])
   music.play()
end


-- Save old data
last_sysFaction = nil
last_sysNebuDens = nil
last_sysNebuVol = nil
-- Choose ambient songs
function choose_ambient ()
   force = true

   -- Check to see if we want to update
   if music.isPlaying() then
      if last == "takeoff" then
         return
      elseif last == "ambient" then
         force = false
      end
   end

   -- Get information about the current system
   sys = space.getSystem()
   factions = sys:faction()
   nebu_dens, nebu_vol = sys:nebulae()

   -- Check to see if changing faction zone
   if not factions[last_sysFaction] then

      -- Table must not be empty
      if next(factions) ~= nil then
         force = true
      end

      if force then
         -- Give first value to last faction
         for k,v in pairs(factions) do
            last_sysFaction = k
            break
         end
      end
   end

   -- Check to see if entering nebulae
   nebu = nebu_dens > 0
   if nebu ~= last_sysNebuDens then
      force = true
      last_sysNebuDens = nebu
   end
 
   -- Must be forced
   if force then
      -- Stop playing first and have this trigger again
      if music.isPlaying() then
         music.stop()
         return
      end

      -- Choose the music, bias by faction first
      if factions["Collective"] then
         ambient = { "collective1" }
      elseif nebu and rnd.int(0,1) == 0 then
         ambient = { "ambient1" }
      else
         ambient = { "ambient2", "mission",
                     "peace1", "peace2", "peace4", "peace6" }
      end

      -- Load music and play
      music.load( ambient[ rnd.int(1,#ambient) ])
      music.play()
   end
end


-- Battle songs
function choose_battle ()
   music.load( "galacticbattle" )
   music.play()
end
