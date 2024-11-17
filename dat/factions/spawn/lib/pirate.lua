local scom = require "factions.spawn.lib.common"
local lf = require "love.filesystem"

local spir = {}

-- TODO integrate this with lib/default
function spir.initDirectory( dir, faction, params )
   params = params or {}
   params.faction = faction

   -- Set up directory
   local spawners = {}
   for k,v in ipairs(lf.getDirectoryItems('factions/spawn/pirate')) do
      local f, priority = require( "factions.spawn.pirate."..string.gsub(v,".lua","") )
      table.insert( spawners, { p=(priority or 5)+1000, f=f } ) -- Generic pirate gets run first
   end
   if dir ~= nil then
      for k,v in ipairs(lf.getDirectoryItems('factions/spawn/'..dir)) do
         local f, priority = require( "factions.spawn."..dir.."."..string.gsub(v,".lua","") )
         table.insert( spawners, { p=priority or 5, f=f } )
      end
   end
   table.sort( spawners, function( a, b )
      return a.p > b.p -- Lower priority gets run later, so it can overwrite
   end )

   -- Create init function (global)
   _G.create = function ( max )
      local spawn = {}
      for k,v in ipairs(spawners) do
         v.f( spawn, max, params )
      end
      -- Transform to old system. TODO replace when done
      local weights = {}
      for k,v in pairs(spawn) do
         weights[ v.f ] = v.w
      end
      -- Clear up empty fields
      for k,v in pairs(weights) do
         if v <= 0 then
            weights[k] = nil
         end
      end
      return scom.init( faction, weights, max, params )
   end
end

return spir
