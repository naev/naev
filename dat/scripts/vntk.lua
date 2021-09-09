--[[
   Small wrapper around vn to do toolkit type stuff.
--]]
local vntk = {}

function vntk.msg( title, text )
   vn.reset()
   vn.scene()
   local c
   if title then
      c = vn.newCharacter( title )
   else
      c = vn.na
   end
   vn.transition()
   c( text )
   vn.done()
end

return vntk
