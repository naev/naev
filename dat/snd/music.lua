--[[
choose will get called with a string parameter indicating status valid
parameters:
   load - game is loading
   land - player landed
   takeoff - player took off
   combat - player just got a hostile onscreen
   idle - current playing music ran out
]]--
local audio = require "love.audio"
local last_track
local tracks = {}
local music_stopped = false
local music_situation = "load"
local music_played = 0
local music_vol = naev.conf().music

local function tracks_stop ()
   for k,v in ipairs( tracks ) do
      v.fade = -1
   end
end

local function tracks_add( name, situation )
   if music_situation ~= situation then
      music_played = 0
   end
   music_situation = situation
   last_track = name

   local name_orig = name
   name = "snd/music/"..name_orig..".ogg"
   if naev.file.filetype( name ) ~= "file" then
      name = "snd/music/"..name_orig
      if naev.file.filetype( name ) ~= "file" then
         name = name_orig
      end
   end

   local m = audio.newSource( name, "stream" )
   m:setVolume( music_vol )
   m:play()
   local t = {
      m     = m,
      fade  = nil,
      vol   = 1,
      name  = name_orig,
   }
   tracks_stop () -- Only play one at a time
   table.insert( tracks, t )
   return t
end

local function tracks_playing ()
   for k,v in ipairs( tracks ) do
      if (not v.fade or v.fade > 0) and not v.m:isStopped() then
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

-- Faction-specific songs.
local factional = {
   Collective = { "collective1", "automat" },
   Pirate     = { "pirate1_theme1", "pirates_orchestra", "ambient4",
                  "terminal" },
   Empire     = { "empire1", "empire2"; add_neutral = true },
   Sirius     = { "sirius1", "sirius2"; add_neutral = true },
   Dvaered    = { "dvaered1", "dvaered2"; add_neutral = true },
   ["Za'lek"] = { "zalek1", "zalek2"; add_neutral = true },
   Thurion    = { "motherload", "dark_city", "ambient1", "ambient3" },
   Proteron   = { "heartofmachine", "imminent_threat", "ambient4" },
}

-- Planet-specific songs
local planet_songs = {
   ["Minerva Station"] = { "meeting_mtfox" },
   ["Strangelove Lab"] = { "landing_sinister" },
   ["One-Wing Goddard"] = { "/snd/sounds/songs/inca-spa.ogg" },
   ["Research Post Sigma-13"] = function ()
         if not diff.isApplied("sigma13_fixed1") and
            not diff.isApplied("sigma13_fixed2") then
            return "landing_sinister"
         end
      end,
}

-- System-specific songs
local system_ambient_songs = {
   Taiomi = { "/snd/sounds/songs/inca-spa.ogg" },
}

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
   return playIfNotPlaying( "machina", "load" )
end

--[[--
Chooses Intro songs.
--]]
function choose_table.intro ()
   return playIfNotPlaying( "intro", "intro" )
end

--[[--
Chooses Credit songs.
--]]
function choose_table.credits ()
   return playIfNotPlaying( "empire1", "credits" )
end

--[[--
Chooses landing songs.
--]]
function choose_table.land ()
   local pnt   = spob.cur()
   local class = pnt:class()
   local music_list

   -- Planet override
   local override = planet_songs[ pnt:nameRaw() ]
   if override then
      if type(override)=="function" then
         local song = override()
         if song then
            tracks_add( song, "land" )
            return true
         end
      else
         tracks_add( override[ rnd.rnd(1, #override) ], "land" )
         return true
      end
   end

   -- Standard to do it based on type of planet
   if class == "M" then
      music_list = { "agriculture" }
   elseif class == "O" then
      music_list = { "ocean" }
   elseif class == "P" then
      music_list = { "snow" }
   else
      if pnt:services()["inhabited"] then
         music_list = { "cosmostation", "upbeat" }
      else
         music_list = { "agriculture" }
      end
   end

   tracks_add( music_list[ rnd.rnd(1, #music_list) ], "land" )
   return true
end


-- Takeoff songs
function choose_table.takeoff ()
   -- No need to restart
   if music_situation == "takeoff" and tracks_playing() then
      return true
   end
   local takeoff = { "liftoff", "launch2", "launch3chatstart" }
   tracks_add( takeoff[ rnd.rnd(1,#takeoff) ], "takeoff" )
   return true
end


-- Save old data
local last_sysFaction  = nil
local last_sysNebuDens = nil
local ambient_neutral  = { "ambient2", "mission",
      "peace1", "peace2", "peace4", "peace6",
      "void_sensor", "ambiphonic",
      "ambient4", "terminal", "eureka",
      "ambient2_5", "78pulse", "therewillbestars" }
--[[--
Chooses ambient songs.
--]]
function choose_table.ambient ()
   local force = true
   local ambient

   -- Check to see if we want to update
   local track = tracks_playing()
   if track then
      if music_situation == "takeoff" then
         return true
      elseif music_situation == "ambient" then
         force = false
      end

      -- Do not change songs too soon
      if track.m:tell() < 10 then
         return false
      end
   end

   -- Get information about the current system
   local sys       = system.cur()
   local factions  = sys:presences()
   local nebu_dens = sys:nebula()

   -- System
   local override = system_ambient_songs[ sys:nameRaw() ]
   if override then
      tracks_add( override[ rnd.rnd(1, #override) ], "ambient" )
      return true
   end

   local strongest = var.peek("music_ambient_force")
   if strongest == nil then
      if factions then
         local strongest_amount = 0
         for k, v in pairs( factions ) do
            if v > strongest_amount then
               strongest = k
               strongest_amount = v
            end
         end
      end
   end

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
      if strongest ~= nil and factional[strongest] then
         ambient = factional[strongest]
         add_neutral = factional[strongest].add_neutral
      elseif nebu then
         ambient = { "ambient1", "ambient3" }
         add_neutral = true
      else
         ambient = ambient_neutral
      end

      -- Clobber array with generic songs if allowed.
      if add_neutral and rnd.rnd() < neutral_prob then
         ambient = ambient_neutral
      end

      -- Make sure it's not already in the list or that we have to stop the
      -- currently playing song.
      if track then
         for k,v in pairs(ambient) do
            if track.name == v then
               return false
            end
         end

         tracks_stop()
         return true
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


-- Faction-specific combat songs
local factional_combat = {
   Collective = { "collective2", "galacticbattle", "battlesomething1", "combat3" },
   Pirate     = { "battlesomething2", "blackmoor_tides", add_neutral = true },
   Empire     = { "galacticbattle", "battlesomething2"; add_neutral = true },
   Goddard    = { "flf_battle1", "battlesomething1"; add_neutral = true },
   Dvaered    = { "flf_battle1", "battlesomething1", "battlesomething2"; add_neutral = true },
   ["FLF"]    = { "flf_battle1", "battlesomething2"; add_neutral = true },
   Frontier   = { "flf_battle1"; add_neutral = true },
   Sirius     = { "galacticbattle", "battlesomething1"; add_neutral = true },
   Soromid    = { "galacticbattle", "battlesomething2"; add_neutral = true },
   ["Za'lek"] = { "collective2", "galacticbattle", "battlesomething1", add_neutral = true }
}

--[[--
Chooses battle songs.
--]]
function choose_table.combat ()
   -- Get some data about the system
   local sys       = system.cur()
   local nebu_dens = sys:nebula()
   local combat

   local strongest = var.peek("music_combat_force")
   if strongest == nil then
      local presences = sys:presences()
      if presences then
         local strongest_amount = 0
         for k, v in pairs( presences ) do
            if faction.get(k):playerStanding() < 0 and v > strongest_amount then
               strongest = k
               strongest_amount = v
            end
         end
      end
   end

   local nebu = nebu_dens > 0
   if nebu then
      combat = { "nebu_battle1", "nebu_battle2", "combat1", "combat2" }
   else
      combat = { "combat3", "combat1", "combat2" }
   end

   if factional_combat[strongest] then
      if factional_combat[strongest].add_neutral then
         for k, v in ipairs( factional_combat[strongest] ) do
            combat[ #combat + 1 ] = v
         end
      else
         combat = factional_combat[strongest]
      end
   end

   -- Make sure it's not already in the list or that we have to stop the
   -- currently playing song.
   local t = tracks_playing()
   if t then
      for k,v in pairs(combat) do
         if t.name == v then
            return true
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
   -- Don't change or play music if a mission or event doesn't want us to
   if var.peek( "music_off" ) then
      return
   end

   -- Allow restricting play of music until a song finishes
   if var.peek( "music_wait" ) then
      if tracks_playing() then
         return
      else
         var.pop( "music_wait" )
      end
   end

   str = str or "ambient"

   choose_table[ str ]()
end

local enemy_dist = 5e3
local function should_combat ()
   local pp = player.pilot()
   if not pp or not pp:exists() then
      return false
   end

   if player.autonav() then
      return false
   end

   if pp:flags("stealth") then
      return false
   end

   if pp:lockon() > 0 then
      choose( "combat" )
      return true
   end

   local enemies = pp:getHostiles( enemy_dist, nil, true )
   for k,v in ipairs(enemies) do
      if v:target() == pp then
         choose( "combat" )
         return true
      end
   end
   return false
end

local function should_ambient ()
   local pp = player.pilot()
   if not pp or not pp:exists() then
      return false
   end

   local enemies = pp:getHostiles( enemy_dist, nil, true )
   if #enemies <= 0 then
      choose( "ambient" )
      return true
   end
   return false
end

local update_rate = 0.5
local update_timer = 0
local update_fade = 1/2
function update( dt )
   local remove = {}
   for k,v in ipairs(tracks) do
      if v.fade then
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
               table.insert( remove, k )
            end
         end
         v.m:setVolume( music_vol * v.vol )
      end
   end
   for k=#remove, 1, -1 do
      table.remove( tracks, k )
   end

   -- Not going to do anything
   if music_stopped then
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
   if not curtrack or music_played > 10 then
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
      choose( "ambient" )
   end
end

function play( song )
   music_stopped = false
   tracks_add( song, "custom" )
end

function stop ()
   tracks_stop()
   music_stopped = true
end

function pause ()
   tracks_pause()
end

function resume ()
   tracks_resume()
end

function info ()
   local t = tracks_playing()
   if not t then
      return false
   end
   return true, t.name, t.m:tell()
end

function volume( vol )
   if not vol then
      return music_vol
   end
   music_vol = vol
   for k,v in ipairs( tracks ) do
      v.m:setVolume( music_vol * v.vol )
   end
end
