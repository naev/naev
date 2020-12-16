--[[
-- Keyboard
--]]
local keyboard = {}
keyboard._keystate = {}
keyboard._repeat = false
-- Internal function that connects to Naev
function keyboard.isDown( key )
   return (keyboard._keystate[ key ] == true)
end
function keyboard.setKeyRepeat( enable )
   keyboard._repeat = enable
end


return keyboard
