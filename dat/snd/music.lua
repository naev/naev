--[[
choose will get called with a string parameter indicating status.
Valid parameters:
   load - game is loading
   land - player landed
   takeoff - player took off
   combat - player just got a hostile onscreen
   idle - current playing music ran out
]]--
local audio = require "love.audio"
local utf8 = require "utf8"
local lf = require "love.filesystem"

-- List of songs to use
local takeoff_songs = {}
local factional_songs = {}
local spob_songs = {}
local system_ambient_songs = {}
local ambient_songs = {}
local loading_songs = {}
local intro_songs = {}
local credits_songs = {}
local factional_combat_songs = {}
local combat_songs = {}
local nebula_combat_songs = {}

for k,v in ipairs(lf.getDirectoryItems("snd/music")) do
   local suffix = utf8.sub(v, -4)
   if suffix=='.lua' then
      local t = require( "snd.music."..string.gsub(v,".lua","") )
      tmerge( factional_songs, t.factional_songs )
      tmerge( spob_songs, t.spob_songs )
      tmerge( system_ambient_songs, t.system_ambient_songs )
      tmerge( takeoff_songs, t.takeoff_songs )
      tmerge( ambient_songs, t.ambient_songs )
      tmerge( loading_songs, t.loading_songs )
      tmerge( intro_songs, t.intro_songs )
      tmerge( credits_songs, t.credits_songs )
      tmerge( factional_combat_songs, t.factional_combat_songs )
      tmerge( combat_songs, t.combat_songs )
      tmerge( nebula_combat_songs, t.nebula_combat_songs )
   end
end

local last_track -- last played track name
local tracks = {} -- currently playing tracks (including fading)
local music_off = false -- disabled picking or changing music
local music_situation -- current running situation
local music_played = 0 -- elapsed play time for the current situation
local music_vol = naev.conf().music -- music global volume

local function tracks_stop ()
   local remove = {}
   for k,v in ipairs( tracks ) do
      v.fade = -1
      v.paused = nil
      if v.elapsed <= 0 or v.m:isPaused() then
         v.m:stop()
         table.insert( remove, k )
      end
   end
   for k=#remove, 1, -1 do
      table.remove( tracks, k )
   end
end

local function tracks_add( name, situation, params )
   params = params or {}
   if music_situation ~= situation then
      music_played = 0
   end
   music_situation = situation
   last_track = name

   local name_orig = name
   if string.sub(name, 1, 1) ~= "/" then
      name = "snd/music/"..name_orig
   end

   local m = audio.newSource( name, "stream" )
   if params.fade then
      m:setVolume( 0, true )
   else
      m:setVolume( music_vol, true )
   end
   local t = {
      m     = m,
      fade  = params.fade,
      vol   = 1,
      delay = params.delay,
      name  = name_orig,
      elapsed = 0,
   }
   if not params.delay then
      m:play()
   end
   tracks_stop() -- Only play one at a time
   table.insert( tracks, t )
   return t
end

local function tracks_playing ()
   for k,v in ipairs( tracks ) do
      if (not v.fade or v.fade > 0) and (v.delay or not v.m:isStopped()) then
         return v
      end
   end
   return nil
end

local function tracks_pause ()
   for k,v in ipairs( tracks ) do
      if not v.fade or v.fade > 0 then
         v.fade = -1
         v.paused = true
      end
   end
end

local function tracks_resume ()
   for k,v in ipairs( tracks ) do
      if v.paused then
         v.fade = 1
         v.paused = nil
         v.m:play()
      end
   end
end

--[[--
Play a song if it's not currently playing.
--]]
local function playIfNotPlaying( song, situation, params )
   local track = tracks_playing()
   if track and track.name == song then
      return false
   end
   tracks_add( song, situation, params )
   return true
end

-- Stores all the available sound types and their functions
local choose_table = {}

--[[--
Chooses Loading songs.
--]]
function choose_table.load ()
   local params = {
      delay = 0.8,
   }
   if not music_situation then
      params.delay = nil
   end
   local s = loading_songs[ rnd.rnd(1,#loading_songs) ]
   return playIfNotPlaying( s, "load", params )
end

--[[--
Chooses Intro songs.
--]]
function choose_table.intro ()
   local s = intro_songs[ rnd.rnd(1,#intro_songs) ]
   return playIfNotPlaying( s, "intro" )
end

--[[--
Chooses Credit songs.
--]]
function choose_table.credits ()
   local s = credits_songs[ rnd.rnd(1,#credits_songs) ]
   return playIfNotPlaying( s, "credits" )
end

--[[--
Chooses landing songs.
--]]
function choose_table.land ()
   if not player.isLanded() then
      return choose_table.ambient()
   end
   local pnt   = spob.cur()
   local class = pnt:class()
   local music_list
   local params = {
      delay = 0.8,
   }

   -- Planet override
   local override = spob_songs[ pnt:nameRaw() ]
   if override then
      if type(override)=="function" then
         local song = override()
         if song then
            if type(song)=="table" then
               tracks_add( song[ rnd.rnd(1, #song) ], "land", params )
            else
               tracks_add( song, "land", params )
            end
            return true
         end
      else
         tracks_add( override[ rnd.rnd(1, #override) ], "land", params )
         return true
      end
   end

   -- Standard to do it based on type of planet
   if class == "M" then
      music_list = { "agriculture.ogg" }
   elseif class == "O" then
      music_list = { "ocean.ogg" }
   elseif class == "P" then
      music_list = { "snow.ogg" }
   else
      if pnt:services()["inhabited"] then
         music_list = { "cosmostation.ogg", "upbeat.ogg" }
      else
         music_list = { "agriculture.ogg" }
      end
   end

   tracks_add( music_list[ rnd.rnd(1,#music_list) ], "land", params )
   return true
end


-- Takeoff songs
function choose_table.takeoff ()
   -- No need to restart
   if music_situation == "takeoff" and tracks_playing() then
      return false
   end
   tracks_add( takeoff_songs[ rnd.rnd(1,#takeoff_songs) ], "ambient" ) -- Don't want to repeat takeoff
   return true
end

local function sys_strongest_faction( sys, combat )
   sys = sys or system.cur()
   local strongest = var.peek("music_ambient_force")
   if strongest == nil then
      local strongest_amount = 0
      for k, v in pairs( sys:presences() ) do
         local f = faction.get(k)
         if f:tags().pirate then
            k = "Pirate" -- We don't distinguish between pirate factions ATM
         end
         if (not combat or sys:reputation(f) < 0) and v > strongest_amount then
            strongest = k
            strongest_amount = v
         end
      end
   end
   return strongest
end

-- Save old data
local last_sysFaction  = nil
local last_sysNebuDens = nil
--[[--
Chooses ambient songs.
--]]
function choose_table.ambient ()
   local force = true
   local ambient

   -- Check to see if we want to update
   local track = tracks_playing()
   if track then
      if music_situation == "ambient" then
         force = false
      end

      -- Do not change songs too soon
      if track.m:tell() < 10 then
         return false
      end
   end

   -- Get information about the current system
   local sys       = system.cur()
   local nebu_dens = sys:nebula()

   -- System
   local override = system_ambient_songs[ sys:nameRaw() ]
   if override then
      tracks_add( override[ rnd.rnd(1,#override) ], "ambient" )
      return true
   end

   local strongest = sys_strongest_faction( sys, false )

   -- Check to see if changing faction zone
   if strongest ~= last_sysFaction then
      force = true

      if force then
         last_sysFaction = strongest
      end
   end

   -- Check to see if entering nebula
   local nebu = nebu_dens > 0
   if nebu ~= last_sysNebuDens then
      force = true
      last_sysNebuDens = nebu
   end

   -- Must be forced
   if force then
      -- Choose the music, bias by faction first
      local add_neutral = false
      local neutral_prob = 0.6
      if strongest ~= nil and factional_songs[strongest] then
         ambient = factional_songs[strongest]
         add_neutral = factional_songs[strongest].add_neutral
      elseif nebu then
         ambient = { "ambient1.ogg", "ambient3.ogg" }
         add_neutral = true
      else
         ambient = ambient_songs
      end

      -- Clobber array with generic songs if allowed.
      if add_neutral and rnd.rnd() < neutral_prob then
         ambient = ambient_songs
      end

      -- Make sure it's not already in the list or that we have to stop the
      -- currently playing song.
      if track then
         for k,v in pairs(ambient) do
            if track.name == v then
               return false
            end
         end
      end

      -- Avoid repetition
      local new_track_id = rnd.rnd(1,#ambient)
      local new_track = ambient[ new_track_id ]
      if new_track == last_track then
         new_track_id = math.fmod( new_track_id, #ambient )+1
         new_track = ambient[ new_track_id ]
      end

      tracks_add( new_track, "ambient" )
      return true
   end

   return false
end


--[[--
Chooses battle songs.
--]]
function choose_table.combat ()
   -- Get some data about the system
   local sys       = system.cur()
   local nebu_dens = sys:nebula()
   local combat

   local strongest = sys_strongest_faction( sys, true )

   local nebu = nebu_dens > 0
   if nebu then
      combat = nebula_combat_songs
   else
      combat = combat_songs
   end

   if factional_combat_songs[strongest] then
      if factional_combat_songs[strongest].add_neutral then
         for k, v in ipairs( factional_combat_songs[strongest] ) do
            combat[ #combat + 1 ] = v
         end
      else
         combat = factional_combat_songs[strongest]
      end
   end

   -- Make sure it's not already in the list or that we have to stop the
   -- currently playing song.
   local t = tracks_playing()
   if t then
      for k,v in pairs(combat) do
         if t.name == v then
            return false
         end
      end
   end

   -- Avoid repetition
   local new_track_id = rnd.rnd(1,#combat)
   local new_track = combat[ new_track_id ]
   if new_track == last_track then
      new_track_id = math.fmod( new_track_id, #combat )+1
      new_track = combat[ new_track_id ]
   end

   tracks_add( new_track, "combat" )
   return true
end

function choose( str )
   -- Allow restricting play of music until a song finishes
   if var.peek( "music_wait" ) then
      if tracks_playing() then
         return
      else
         var.pop( "music_wait" )
      end
   end

   str = str or music_situation
   local choose_func = choose_table[ str ]
   if not choose_func then
      choose_func = choose_table.ambient
   end
   choose_func()
end

local enemy_dist = 5e3
--[[
See if we should switch to playing combat music.
--]]
local function should_combat ()
   local pp = player.pilot()
   if not pp or not pp:exists() then
      return false
   end

   -- Enforce minimum play time
   if music_played < 10 then
      return false
   end

   -- Don't play combat music in autonav
   if player.autonav() then
      return false
   end

   -- Don't play combat in stealth
   -- TODO stealth effects or music?
   if pp:flags("stealth") then
      return false
   end

   -- If locked in, time to switch
   if pp:lockon() > 0 then
      choose( "combat" )
      return true
   end

   -- Nearby enemies targeting the player will also switch
   local enemies = pp:getEnemies( enemy_dist )
   for k,v in ipairs(enemies) do
      local tgt = v:target()
      if tgt and tgt:withPlayer() then
         choose( "combat" )
         return true
      end
   end
   return false
end

--[[
See if we should switch to playing ambient music again.
--]]
local function should_ambient ()
   local pp = player.pilot()
   if not pp or not pp:exists() then
      return false
   end

   -- Enforce minimum play time
   if music_played < 10 then
      return false
   end

   -- Enemies nearby
   local enemies = pp:getEnemies( enemy_dist )
   if #enemies > 0 then
      return false
   end

   -- Still locked on
   if pp:lockon() > 0 then
      return false
   end

   choose( "ambient" )
   return true
end

local update_rate = 0.5
local update_timer = 0
local update_fade = 1 -- Inverse of the time to fade out
function update( dt )
   dt = math.min( dt, 0.1 ) -- Take smaller steps when lagging
   local remove = {}
   for k,v in ipairs(tracks) do
      v.elapsed = v.elapsed + dt
      if v.delay then
         v.delay = v.delay - dt
         if v.delay < 0 then
            v.delay = nil
            v.m:play()
         end
      elseif v.fade then
         v.vol = v.vol + v.fade * update_fade * dt
         if v.vol > 1 then
            v.vol = 1
            v.fade = nil
         elseif v.vol < 0 then
            v.vol = 0
            v.fade = nil
            if v.paused then
               v.m:pause()
            else
               v.m:stop()
               table.insert( remove, k )
               v = nil
            end
         end
         if v then
            v.m:setVolume( music_vol * v.vol, true )
         end
      end
   end
   for k=#remove, 1, -1 do
      table.remove( tracks, k )
   end

   -- Not going to do anything
   if music_off then
      return
   end

   -- Limit executions a bit
   update_timer = update_timer - dt
   music_played = music_played + dt
   if update_timer > 0 then
      return
   end
   update_timer = update_rate

   -- If paused just wait
   local curtrack = tracks_playing()
   if curtrack and curtrack.paused then
      return -- don't do anything while paused
   end

   -- See if we should spice up the music a bit
   if not tk.isOpen() and (not curtrack or music_played > 10) then
      if music_situation == "ambient" then
         if should_combat() then
            return
         end
      elseif music_situation == "combat" then
         if should_ambient() then
            return
         end
      end
   end

   -- No track, so we choose a random ambient track
   if not curtrack then
      choose()
   end
end

function play( song )
   music_off = false
   if song then
      tracks_add( song, "custom" )
      return
   end
   tracks_resume()
   local t = tracks_playing()
   if not t then
      choose()
   end
end

function stop( disable )
   tracks_stop()
   if disable then
      music_off = true
   end
end

function pause( disable )
   tracks_pause()
   if disable then
      music_off = true
   end
end

function info ()
   local t = tracks_playing()
   if not t then
      return false
   end
   return true, t.name, t.m:tell()
end

function volume( vol )
   music_vol = vol
   for k,v in ipairs( tracks ) do
      v.m:setVolume( music_vol * v.vol, true )
   end
end
