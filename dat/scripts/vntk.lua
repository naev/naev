--[[
   Small wrapper around vn to do toolkit type stuff.
--]]
local vn = require "vn"
local vntk = {}

function vntk.msg( title, text )
   if type(text) ~= "table" then
      text = {text}
   end

   vn.reset()
   vn.scene()
   local c
   if title and title ~= "" then
      c = vn.newCharacter( title )
   else
      c = vn.na
   end
   vn.transition( "blur", 0.2 )
   for k,t in ipairs(text) do
      c( t )
   end
   vn.done( "blur", 0.2 )
   vn.run()
end

return vntk
