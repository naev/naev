--[[
   Small wrapper around vn to do toolkit type stuff.
--]]
local vn = require "vn"
local vntk = {}

function vntk.msg( title, text )
   vn.reset()
   vn.scene()
   local c
   if title and title ~= "" then
      c = vn.newCharacter( title )
   else
      c = vn.na
   end
   vn.transition()
   c( text )
   vn.run()
end

return vntk
