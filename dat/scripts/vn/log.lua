local graphics = require 'love.graphics'

local log = {}

local _log = {}

function log.reset ()
   _log = {}
end

function log.add( entry )
   table.insert( _log, entry )
end

function log.draw ()
   graphics.setColor( 0, 0, 0, 0.8 )
   local lw, lh = graphics.getDimensions()
   graphics.rectangle( "fill", 0, 0, lw, lh )
end

function log.update ()
end

return log
