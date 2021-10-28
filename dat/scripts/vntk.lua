--[[
   Small wrapper around vn to do toolkit type stuff.
--]]
local vn = require "vn"
local vntk = {}

function vntk.msg( title, text, params )
   params = params or {}
   local transition = params.transition or {"blur",0.2}
   if type(text) ~= "table" then
      text = {text}
   end

   vn.reset()
   vn.scene()
   if params.pre then params.pre() end
   local c
   if title and title ~= "" then
      c = vn.newCharacter( title )
   else
      c = vn.na
   end
   vn.transition( table.unpack(transition) )
   for _k,t in ipairs(text) do
      c( t )
   end
   if params.post then params.post() end
   vn.done( table.unpack(transition) )
   vn.run()
end

function vntk.yesno( title, text, params )
   params = params or {}
   local transition = params.transition or {"blur",0.2}
   if type(text) ~= "table" then
      text = {text}
   end
   local menu = params.menu or 1
   local option

   vn.reset()
   vn.scene()
   if params.pre then params.pre() end
   local c
   if title and title ~= "" then
      c = vn.newCharacter( title )
   else
      c = vn.na
   end
   vn.transition( table.unpack(transition) )
   for k,t in ipairs(text) do
      c( t )
      if k==menu then
         vn.menu{ 
            {_("Yes"), "yes"},
            {_("No"), "no"},
         }
         vn.label("yes")
         vn.func( function () option = true end )
         vn.jump("cont")
         vn.label("no")
         vn.func( function () option = false end )
         vn.label("cont")
      end
   end
   if params.post then params.post() end
   vn.done( table.unpack(transition) )
   vn.run()
   return option
end

return vntk
