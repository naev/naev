--[[
   Thurion Helpers
--]]
local vn = require "vn"

local thurion = {}

function thurion.vn_drone( name, params )
   return vn.Character.new( name, tmerge( {
      image="thurion_drone.webp",
   }, params) )
end

return thurion
