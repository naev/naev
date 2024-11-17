--[[
--]]
require 'ai.pers'
require 'ai.core.misc.distress'

mem.distressmsg = nil -- should be defined

mem.distressmsgfunc = function( target )
   ai.settarget( target )
   ai.distress( mem.distressmsg )
end
