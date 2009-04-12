
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

   elseif str == "intro" then

      choose_intro()

   elseif str == "credits" then

      choose_credits()

   elseif str == "land" then

      choose_land()

   elseif str == "takeoff" then

      choose_takeoff()

   elseif str == "ambient" then

      choose_ambient()

   elseif str == "combat" then

      choose_combat()

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


function checkIfPlayingOrStop( song )
   if music.isPlaying() then
      if music.current() ~= song then
         music.stop()
      end
      return true
   end
   return false
end


-- Loading songs.
function choose_load ()
   load_song = "machina"
   -- Don't play again if needed
   if checkIfPlayingOrStop( load_song ) then
      return
   end
   music.load( load_song )
   music.play()
end


-- Intro music.
function choose_intro ()
   intro_song = "intro"
   -- Don't play again if needed
   if checkIfPlayingOrStop( intro_song ) then
      return
   end
   music.load( intro_song )
   music.play()
end


-- Credits music.
function choose_credits ()
   credits_song = "empire1"
   -- Don't play again if needed
   if checkIfPlayingOrStop( credits_song ) then
      return
   end
   music.load( credits_song )
   music.play()
end


-- Landing songs
function choose_land ()
   pnt = planet.get()
   class = pnt:class()

   if class == "M" then
      mus = { "agriculture" }
   elseif name == "Anecu" then -- TODO we need a way to differenciate aquatics
      mus = { "ocean" }
   elseif class == "P" then
      mus = { "snow" }
   else
      if pnt:services() > 0 then
         mus = { "cosmostation", "upbeat" }
      else
         mus = { "agriculture" }
      end
   end

   music.load( mus[ rnd.rnd(1, #mus) ] )
   music.play()
end


-- Takeoff songs
function choose_takeoff ()
   -- No need to restart
   if last == "takeoff" and music.isPlaying() then
      return
   end
   takeoff = { "liftoff", "launch2", "launch3chatstart" }
   music.load( takeoff[ rnd.rnd(1,#takeoff) ])
   music.play()
end


-- Save old data
last_sysFaction = nil
last_sysNebuDens = nil
last_sysNebuVol = nil
ambient_neutral = { "ambient2", "mission",
      "peace1", "peace2", "peace4", "peace6" }
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
   sys = system.get()
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
      -- Choose the music, bias by faction first
      add_neutral = false
      if factions["Collective"] then
         ambient = { "collective1" }
      elseif factions["Empire"] then
         ambient = { "empire2", "empire2", "empire2",
               "empire1", "empire1", "empire1" }
         add_neutral = true
      elseif nebu then
         ambient = { "ambient1", "ambient1", "ambient1",
               "ambient3", "ambient3", "ambient3" }
         add_neutral = true
      else
         ambient = ambient_neutral
      end

      -- Check if needs to append neutral ambient songs
      if add_neutral then
         for k,v in pairs(ambient_neutral) do
            table.insert(ambient, v)
         end
      end

      -- Make sure it's not already in the list or that we have to stop the
      -- currently playing song.
      if music.isPlaying() then
         cur = music.current()
         for k,v in pairs(ambient) do
            if cur == v then
               return
            end
         end

         music.stop()
         return
      end

      -- Load music and play
      music.load( ambient[ rnd.rnd(1,#ambient) ] )
      music.play()
   end
end


-- Battle songs
function choose_combat ()
   -- Stop music first, but since it'll get saved it'll run this next
   if music.isPlaying() then
      music.stop()
      return
   end

   -- Get some data about the system
   sys = system.get()
   nebu_dens, nebu_vol = sys:nebulae()

   nebu = nebu_dens > 0
   if nebu then
      combat = { "nebu_battle1", "nebu_battle2" }
   else
      combat = { "galacticbattle", "flf_battle1" }
   end

   music.load( combat[ rnd.rnd(1,#combat) ] )
   music.play()
end

