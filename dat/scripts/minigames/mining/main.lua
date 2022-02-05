--local mining = require "minigames.mining.core"
local mining = require "core"

love.load = mining.load
love.draw = mining.draw
love.update = mining.update
love.keypressed = mining.keypressed
