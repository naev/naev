--[[---
Small music handling library.

Based on Love2D and Naev API
@module lmusic
--]]
local lmusic = {
   _sources = {},
}

local love_audio = require "love.audio"

local function _getmusic( m )
   for k,v in pairs(lmusic._sources) do
      if v.filename==m then
         return v
      end
   end
   warn(string.format(_("lmusic: Trying to find music '%s', but it is not playing!"), m))
end

local function _apply( m, f )
   if m==nil then
      for k,v in pairs(lmusic._sources) do
         f(v)
      end
   else
      if type(m)=="string" then
         local v = _getmusic(m)
         if v then
            f(v)
         end
      else
         f(m)
      end
   end
end

--[[---
Plays a music with fade-in and fade-out.

   @tparam string|file filename Name or file object to open.
   @tparam tab params Parameter table.
   @treturn tab New music object.
--]]
function lmusic.play( filename, params )
   params = params or {}
   local p = {
      fadein = 2,
      fadeout = 1,
      looping = true,
      pitch = 1,
      volume = 1,
      effect = nil,
   }
   for k,v in pairs(params) do
      p[k] = v
   end

   local source = love_audio.newSource( filename )
   source:setLooping( p.looping )
   source:setPitch( p.pitch )
   source:setVolume( 0 )
   if p.effect then
      source:setEffect( p.effect )
   end
   source:play()

   local m = {
      filename = filename,
      source = source,
      state = "fadein",
      progress = 0,
      p = p,
   }
   table.insert( lmusic._sources, m )
   return m
end

function lmusic.setPitch( m, pitch )
   _apply( m, function (v)
      v.source:setPitch( pitch )
   end )
end

function lmusic.setVolume( m, vol )
   local mvol = naev.music.getVolume(true)
   _apply( m, function (v)
      v.source:setVolume( vol * mvol, true )
   end )
end

function lmusic.update( dt )
   local remove = {}
   local mvol = naev.music.getVolume(true)
   for k,m in pairs(lmusic._sources) do
      if m.state=="fadein" then
         m.progress = m.progress + dt / m.p.fadein
         local v = m.progress * m.p.volume
         if m.progress >= 1 then
            m.state = "play"
            m.progress = 0
            v = m.p.volume
         end
         m.source:setVolume( v * mvol, true )
      elseif m.state=="fadeout" then
         m.progress = m.progress + dt / m.p.fadeout
         local v = (1-m.progress) * m.p.volume
         if m.progress >= 1 then
            m.source:stop()
            m.state = "stop"
         else
            m.source:setVolume( v * mvol, true )
         end
      end

      -- Check stopped
      if m.source:isStopped() then
         table.insert( remove, k )
      end
   end
   for k,v in ipairs(remove) do
      lmusic._sources[v] = nil
   end
end

--[[---
Stops a playing music (or all of them)

   @tparam tab|nil m Music to stop, or nil to stop all playing music.
--]]
function lmusic.stop( m )
   _apply( m, function (v)
      if v.state ~= "fadeout" then
         v.state = "fadeout"
         v.progess = 0
      end
   end )
end

--[[---
Forcibly stops playing music and restarts Naev's game music.
--]]
function lmusic.clear()
   for k,m in pairs(lmusic._sources) do
      m.source:stop()
   end
   lmusic._sources = {}

   if not music.isPlaying() then
      music.play()
   end
end

return lmusic
