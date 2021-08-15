--[[
-- Font
--]]
local class = require 'class'
local data = require 'love.data'

local font = {}

font.FontData = class.inheritsFrom( data.Data )
font.FontData._type = "FontData"

return font
