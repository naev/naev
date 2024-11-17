--[[

   FLF mission common functions.

--]]
local pir = require "common.pirate"

local flf = {}

-- Get a random system with FLF presence.
function flf.getSystem ()
   local choices = {}
   for i, j in ipairs( system.getAll() ) do
      local p = j:presences()
      if p["FLF"] then
         choices[#choices + 1] = j:nameRaw()
      end
   end
   return system.get(choices[rnd.rnd(1, #choices)])
end


-- Get a system generally good for an FLF mission.
-- These are systems which have both FLF and Dvaered presence.
function flf.getTargetSystem ()
   local choices = {}
   for i, j in ipairs( system.getAll() ) do
      local p = j:presences()
      if p["FLF"] and p["Dvaered"] then
         choices[#choices + 1] = j:nameRaw()
      end
   end
   return system.get(choices[rnd.rnd(1, #choices)])
end


-- Get a system with both FLF and Empire presence.
function flf.getEmpireSystem ()
   local choices = {}
   for i, j in ipairs( system.getAll() ) do
      local p = j:presences()
      if p["FLF"] and p["Empire"] then
         choices[#choices + 1] = j:nameRaw()
      end
   end
   return system.get(choices[rnd.rnd(1, #choices)])
end


-- Get a system with both FLF and Pirate presence.
function flf.getPirateSystem ()
   local choices = {}
   for i, j in ipairs( system.getAll() ) do
      local p = j:presences()
      if p[ "FLF" ] and pir.systemPresence( j ) then
         choices[#choices + 1] = j:nameRaw()
      end
   end
   return system.get(choices[rnd.rnd(1, #choices)])
end


-- Change the reputation cap for the FLF.
function flf.setReputation( newcap )
   var.push( "_fcap_flf", math.max(newcap, var.peek("_fcap_flf") or 5) )
end


-- Add an entry to the FLF campaign log.
function flf.addLog( text )
   shiplog.create( "flf", _("FLF"), _("Frontier") )
   shiplog.append( "flf", text )
end

return flf
