--[[
   Support common functions for Points of Interest
--]]
local poi = {}

function poi.setup( sys, risk, reward )
   sys = system.get( sys )
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
