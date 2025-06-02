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

   local preprocess = params.preprocess
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
      end

      -- Nerf the pilot as necessary
      params.preprocess = function( pms )
         pms.intrinsics = pms.intrinsics or {}

         -- Marauders can be further handicapped randomly
         local hcp = handicap
         if total_presence < ((chapter0 and 500) or 300) and rnd.rnd() < 0.5 then
            hcp = hcp+1
         end

         -- Apply as intrinsics
         if hcp>=3 then
            table.insert( pms.intrinsics, HANDICAP3 )
         elseif hcp==2 then
            table.insert( pms.intrinsics, HANDICAP2 )
         elseif hcp==1 then
            table.insert( pms.intrinsics, HANDICAP1 )
         end

         -- Propagate if necessary
         if preprocess then
            preprocess( pms )
         end
      end

      -- Give treasure maps based on chance
      params.postprocess = function( p )
         local chance = p:ship():size()+1 / 8
         if p:faction()==FMARAUDER then
            chance = chance * 0.1
         end
         if rnd.rnd() < chance then
            local pm = p:memory()
            if not pm.lootables then
               pm.lootables = {}
            end
            table.insert( pm.lootables, "treasure_map" )
         end
         if postprocess then
            postprocess(p)
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
