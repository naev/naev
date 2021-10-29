--[[--
Small wrapper around vn to do toolkit type stuff. Is similar to the builtin tk module but with much more functionality.

@module vntk
--]]
local vn = require "vn"
local vntk = {}

--[[--
Creates a series of message boxes to be displayed.

   @tparam[opt] string title Title of the message boxes.
   @tparam string|table text String or table of strings for the text of each consecutive text box.
   @tparam table params Table of parameters
--]]
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

--[[--
Creates a series of message boxes with a yes or no prompt to be displayed.

   @tparam[opt] string title Title of the message boxes.
   @tparam string|table text String or table of strings for the text of each consecutive text box.
   @tparam table params Table of parameters
   @treturn boolean true if yes was pressed or false otherwise.
--]]
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
