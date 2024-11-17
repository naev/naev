--[[
-- Keyboard
--]]
local keyboard = {}
keyboard._keystate = {}
keyboard._repeat = false

-- Internal function that connects to Naev
function keyboard.isDown( ... )
   for k,key in ipairs{...} do
      if (keyboard._keystate[ key ] == true) then
         return true
      end
   end
   return false
end
function keyboard.setKeyRepeat( enable )
   keyboard._repeat = enable
end

-- One to one mapping with Naev API
keyboard.setTextInput = naev.setTextInput
keyboard.hasTextInput = naev.hasTextInput

return keyboard
