--[[
-- Audio
--]]
local class = require 'class'
local object = require 'love.object'

local audio = {}
audio.Source = class.inheritsFrom( object.Object )
audio.Source._type = "Source"
function audio.newSource( filename, type )
   love._unimplemented()
   local s = audio.Source.new()
   --s.a = audio.new( filename, type )
   return s
end
function audio.Source:play() return true end
function audio.Source:pause() end
function audio.Source:stop() end
function audio.Source:isPlaying() return false end 
function audio.Source:setVolume( volume ) end
function audio.Source:getVolume() return audio.getVolume() end
function audio.Source:setLooping( looping ) end
function audio.Source:setPitch( pitch ) end
function audio.Source:setPosition( x, y, z ) end
function audio.Source:setAttenuationDistances( ref, max ) end
function audio.setVolume( volume ) end -- Don't allow setting master volume
function audio.getVolume( volume )
   return audio.getVolume()
end
function audio.setPosition( x, y, z ) end


return audio
