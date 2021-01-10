--[[
-- Audio
--]]
local class = require 'class'
local love = require 'love'
local object = require 'love.object'

local love_audio = {}
function love_audio.newSource( filename, type )
   return naev.audio.new( filename )
end
function love_audio.setVolume( volume ) end -- Don't allow setting master volume
function love_audio.getVolume( volume )
   return naev.audio.getVolume()
end
function love_audio.setPosition( x, y, z ) end -- Don't allow setting position


return love_audio
