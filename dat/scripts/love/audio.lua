--[[
-- Audio
--]]

local love_audio = {}
function love_audio.newSource( filename, _type )
   -- We actually have most of the API implemented on a C level.
   return naev.audio.new( filename )
end
function love_audio.setVolume( _volume ) end -- Don't allow setting master volume
function love_audio.getVolume()
   return naev.audio.getVolume()
end
function love_audio.setPosition( _x, _y, _z ) end -- Don't allow setting position
function love_audio.setEffect( ... )
   return naev.audio.setEffect( ... )
end


return love_audio
