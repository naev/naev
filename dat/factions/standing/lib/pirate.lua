local pir = require 'common.pirate'
local fpir = require 'factions.pirate'
local sbase = require "factions.standing.lib.base"

local spir = {}
friendly_at = 40 -- Lower default than sbase

function spir.init( args )
   args = tmerge( fpir, args or {} ) -- Not recursive to overwrite
   return sbase.init( args )
end

-- Override hit function
local oldhit = hit
function hit( current, amount, source, secondary )
   local value = math.max( -50, oldhit( current, amount, source, secondary ) )

   -- Get the maximum player value with any pirate clan
   local maxval = value
   for k,v in ipairs(pir.factions_clans) do
      if v ~= sbase.fct then
         local vs = v:playerStanding() -- Only get first parameter
         maxval = math.max( maxval, vs )
      end
   end

   -- Update pirate and marauder standings
   pir.updateStandings( maxval )

   -- Set current faction standing
   return value
end

return spir
