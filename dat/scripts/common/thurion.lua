--[[
   Thurion Helpers
--]]
local vn = require "vn"

local thurion = {}

thurion.prefix = "#n".._("THURION: ").."#0" -- Repeatable mission prefix

thurion.liao = {
   portrait = "liao.webp",
   image = "liao.webp",
   name = _("Liao"),
   colour = nil,
   transition = "hexagon",
}

function thurion.vn_drone( name, params )
   return vn.Character.new( name, tmerge( {
      image="thurion_drone.webp",
   }, params) )
end

function thurion.vn_liao( params )
   return vn.Character.new( thurion.liao.name, tmerge( {
      image = thurion.liao.image,
      colour = thurion.liao.colour,
   }, params) )
end

return thurion
