--[[
-- Generic Object template
--
-- Not for use directly
--]]
local class = require 'class'

local object = {}

object.Object = class.inheritsFrom( nil )
object.Object._type = "Object"
function object.Object:type() return self._type end
function object.Object:typeOf( name ) return self._type==name end

return object

