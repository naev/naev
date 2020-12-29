--[[
-- Mouse
--]]
local love = require 'love'

local mouse = {}
mouse.x = 0
mouse.y = 0
mouse.lx = 0
mouse.ly = 0
mouse.down = {}
function mouse.getX() return mouse.x end
function mouse.getY() return mouse.y end
function mouse.isDown( button ) return mouse.down[button]==true end
function mouse.setVisible( visible )
   love._unimplemented()
end

return mouse
