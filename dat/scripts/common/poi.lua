--[[
   Support common functions for Points of Interest
--]]
local lmisn = require "lmisn"
local poi = {}

--[[
   @brief Tries to generate a new setting for a point of interest.
--]]
function poi.generate()
   local syscand = lmisn.getSysAtDistance( nil, 1, 5, function( sys )
      -- Want no inhabited spobs
      for k,p in ipairs(sys:spobs()) do
         if sys.land and sys.inhabitable then
            return false
         end
      end
      return true
   end )

   -- Didn't find system
   if #syscand<=0 then return end

   local sys = syscand[ rnd.rnd(1,#syscand) ]
   -- TODO do something with risk and reward
   local risk = 0
   local reward = 0
   return {
      sys = sys,
      risk = risk,
      reward = reward,
   }
end

function poi.setup( params )
   local risk = params.risk or 0
   local reward = params.reward or 0
   local sys = system.get( params.sys )

   if var.peek("_poi_system") ~= nil or var.peek("_poi_risk") ~= nil or var.peek("_poi_reward") ~= nil then
      warn(_("Point of Interest variables being overwritten!"))
   end

   var.push( "_poi_system", sys:nameRaw() )
   var.push( "_poi_risk", risk )
   var.push( "_poi_reward", reward )
end

function poi.start ()
   local sys = var.peek("_poi_system")
   local risk = var.peek("_poi_risk")
   local reward = var.peek("_poi_reward")
   if sys==nil or risk==nil or reward==nil then
      warn(_("Point of Interest not properly initialized!"))
   end
   if sys ~= nil then
      sys = system.get( sys )
   end
   return sys, risk, reward
end

return poi
