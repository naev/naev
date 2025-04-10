local scom = require "factions.spawn.lib.common"
local lf = require "love.filesystem"
local pir = require "common.pirate"

local spir = {}

local FMARAUDER = faction.get("Marauder")

local HANDICAP1 = outfit.get("Defective") -- Minor malus
local HANDICAP2 = outfit.get("Bad Shape") -- Bad malus
local HANDICAP3 = outfit.get("Worn Down") -- Malus terriblis

-- TODO integrate this with lib/default
function spir.initDirectory( dir, faction, params )
   local isclan = pir.factionIsClan( faction )

   params = params or {}
   params.faction = faction

   -- Set up directory
   local spawners = {}
   -- Load default pirate scripts
   for k,v in ipairs(lf.getDirectoryItems('factions/spawn/pirate')) do
      local f, priority = require( "factions.spawn.pirate."..string.gsub(v,".lua","") )
      table.insert( spawners, { p=(priority or 5)+1000, f=f } ) -- Generic pirate gets run first
   end
   -- Overwrite the pirate scripts with faction-specific ones (if applicable)
   if dir ~= nil then
      for k,v in ipairs(lf.getDirectoryItems('factions/spawn/'..dir)) do
         local f, priority = require( "factions.spawn."..dir.."."..string.gsub(v,".lua","") )
         table.insert( spawners, { p=priority or 5, f=f } )
      end
   end
   table.sort( spawners, function( a, b )
      return a.p > b.p -- Lower priority gets run later, so it can overwrite
   end )

   local postprocess = params.postprocess
   -- Create init function (global)
   _G.create = function ( max )
      local chapter = player.chapter() or "0"
      local chapter0 = chapter=="0"

      -- Some special properties based on the system
      local handicap = 0
      local total_presence = pir.systemPresence()
      if total_presence < ((chapter0 and 150) or 100) then
         handicap = handicap+2
      elseif total_presence < ((chapter0 and 300) or 200) then
         handicap = handicap+1
      end
      -- Marauders are terribad
      if faction==FMARAUDER then
         handicap = handicap+1
         if total_presence < ((chapter0 and 500) or 300) and rnd.rnd() < 0.5 then
            handicap = handicap+1
         end
      end

      -- Nerf the pilot as necessary
      -- TODO this should be done as preprocessing, and not post-processing,
      -- but pilot.add is not flexible enough and hard to change for now.
      params.postprocess = function( p )
         if handicap>=3 then
            p:outfitAddIntrinsic( HANDICAP3 )
         elseif handicap==2 then
            p:outfitAddIntrinsic( HANDICAP2 )
         elseif handicap==1 then
            p:outfitAddIntrinsic( HANDICAP1 )
         end
         -- Propagate postprocess if necessary
         if postprocess then
            postprocess( p )
         end
      end

      -- Clan pirates and really handicapped pirates turn into marauders
      local spawn_faction = faction
      if handicap >= 2 and (handicap > 0 and isclan) then
         spawn_faction = FMARAUDER
      end

      -- Build spawn tables
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
      return scom.init( spawn_faction, weights, max, params )
   end
end

return spir
