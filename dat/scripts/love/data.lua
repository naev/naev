--[[
-- Data
--]]
local class = require 'class'
local object = require 'love.object'

local data = {}

data.Data = class.inheritsFrom( object.Object )
data.Data._type = "Data"

function data.Data:getSize()
   return self.d:getSize()
end
function data.Data:getString()
   return self.d:getString()
end

return data
