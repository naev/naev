--[[
-- Font
--]]
local class = require 'class'
local data = require 'love.data'

local font = {}

font.Font = class.inheritsFrom( data.Data )
font.Font._type = "Font"

return font
