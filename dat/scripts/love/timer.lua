--[[
-- Timer
--]]
local timer = {}
timer._dt = 0
timer._adt = 0
timer._edt = 0
function timer.getDelta() return timer._dt end
function timer.getAverageDelta() return timer._adt end
function timer.getFPS() return 1/timer._adt end
function timer.getTime() return timer._edt end
function timer.sleep( s ) end -- can't really support properly
-- Dummy game-defined functions
function love.update( dt ) end -- dummy


return timer
