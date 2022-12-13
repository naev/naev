--[[
Library to handle avoiding enemies and not being suicidal in general.
--]]
local lanes = require 'ai.core.misc.lanes'

if __ai then
   mem.lanedistance = mem.lanedistance or 2e3
   mem.spobdistance = mem.spobdistance or 3e3
   mem.jumpdistance = mem.jumpdistance or 1e3
end

local careful = {}

local function checkSpobJumps( fct, pos, spobdistance, jumpdistance )
   local scur = system.cur()
   local spobdist2 = math.pow( spobdistance, 2 )
   for k,v in ipairs(scur:spobs()) do
      local f = v:faction()
      if f and f:areEnemies( fct ) then
         if v:pos():dist2( pos ) < spobdist2 then
            return false
         end
      end
   end

   local jumpdist2 = math.pow( jumpdistance, 2 )
   for k,v in ipairs(scur:jumps()) do
      local f = v:dest():faction()
      if f and f:areEnemies( fct ) then
         if v:pos():dist2( pos ) < jumpdist2 then
            return false
         end
      end
   end
   return true
end

--[[--
Checks to see if a position is good for a pilot, considering both lanes and spobs.
--]]
function careful.posIsGood( p, pos )
   local m = p:memory()
   local thr = math.pow( m.lanedistance, 2 )
   local ld = lanes.getDistance2P( p, pos )
   if (ld < math.huge and ld > thr) or lanes.getDistance2PH( p, pos ) < thr then
      return false
   end
   return checkSpobJumps( p:faction(), pos, m.spobdistance, m.jumpdistance )
end

function careful.posIsGoodL( L, fct, pos, lanedistance, spobdistance, jumpdistance )
   lanedistance = lanedistance or 2e3
   spobdistance = spobdistance or lanedistance
   jumpdistance = jumpdistance or spobdistance
   local lanedist = lanes.getDistance2( L, pos )
   if lanedist < math.pow( lanedistance, 2 ) then
      return false
   end
   return checkSpobJumps( fct, pos, lanedistance, spobdistance, jumpdistance )
end

-- Fuses t2 into t1 avoiding duplicates
local function __join_tables( t1, t2 )
   local t = t1
   for k,v in ipairs(t2) do
      local found = false
      for i,u in ipairs(t1) do
         if u==v then
            found = true
            break
         end
      end
      if not found then
         table.insert( t, v )
      end
   end
   return t
end

-- Estimate the strength of a group of pilots
local function __estimate_strength( pilots )
   local str = 0
   for k,p in ipairs(pilots) do
      str = str + p:ship():points()
   end
   -- Diminishing returns for large strengths
   -- ((x+1)**(1-n) - 1)/(1-n)
   local n = 0.3
   return (math.pow(str+1, 1-n) - 1) / (1-n)
   --return str
end

-- See if a target is vulnerable
function careful.checkVulnerable( p, plt, threshold )
   local always_yes = (mem.vulnignore or not mem.natural)
   local pos = plt:pos()
   -- Make sure not in safe lanes
   if always_yes or careful.posIsGood( p, pos ) then
      -- Check to see vulnerability
      local H = 1+__estimate_strength( p:getEnemies( mem.vulnrange, pos ) )
      local F = 1+__estimate_strength( __join_tables(
            p:getAllies( mem.vulnrange, pos, true ),
            p:getAllies( mem.vulnrange, nil, true ) ) )

      if always_yes or F*threshold >= H then
         return true, F, H
      end
   end
   return false
end

local function correctSafePoint( candidate, fct, spobdistance, jumpdistance )
   spobdistance = spobdistance or 3e3
   jumpdistance = jumpdistance or 1e3

   -- Bias away from spobs
   local scur = system.cur()
   local spobdist2 = math.pow( spobdistance, 2 )
   for k,v in ipairs(scur:spobs()) do
      local f = v:faction()
      if not fct or (f and f:areEnemies( fct )) then
         local vp = v:pos()
         if vp:dist2( candidate ) < spobdist2 then
            local off = candidate-vp
            local mod, _dir = off:polar()
            -- This is a really lazy approach, has to be done better
            candidate = candidate + off * spobdistance / mod
         end
      end
   end

   local jumpdist2 = math.pow( jumpdistance, 2 )
   for k,v in ipairs(scur:jumps()) do
      local f = v:dest():faction()
      if not fct or (f and f:areEnemies( fct )) then
         local vp = v:pos()
         if vp:dist2( candidate ) < jumpdist2 then
            local off = candidate-vp
            local mod, _dir = off:polar()
            -- This is a really lazy approach, has to be done better
            candidate = candidate + off * jumpdistance / mod
         end
      end
   end

   return candidate
end

function careful.getSafePoint( p, pos, rad, margin, biasdir )
   -- Try to find a non-lane point
   local candidate = lanes.getNonPointP( p, pos, rad, margin, biasdir )
   if not candidate then
      return nil
   end

   -- Bias away from spobs
   return correctSafePoint( candidate, p:faction(), mem.spobdistance, mem.jumpdistance )
end

function careful.getSafePointL( L, fct, pos, rad, marginlane, marginspob, marginjump )
   -- Try to find a non-lane point
   local candidate = lanes.getNonPoint( L, pos, rad, marginlane )
   if not candidate then
      return nil
   end

   -- Bias away from spobs
   return correctSafePoint( candidate, fct, marginspob, marginjump )
end

return careful
