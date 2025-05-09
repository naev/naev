--[[
--]]
require 'ai.pers'
require 'ai.core.misc.distress'

mem.aggressive = false -- Don't want to fight

mem.distressmsg = nil -- should be defined

mem.distressmsgfunc = function( target )
   ai.settarget( target )
   ai.distress( mem.distressmsg )
end
