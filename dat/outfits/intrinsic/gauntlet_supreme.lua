local fmt = require "format"

notactive = true

local DAMAGEBONUS = 4
local DAMAGETOTAL = 7

function descextra( _p, _o )
   return "#o"..fmt.f(_([[Additional +{bonus}% damage (for a total of {total}%) when using unguided launchers.]]),
      {bonus=DAMAGEBONUS, total=DAMAGETOTAL}).."#0"
end

local function update_damage( p, po )
   local hasunguided = false
   for k,o in ipairs(p:outfits()) do
      if o and o:typeBroad()=="Launcher" then
         local _dps, _disable, _eps, _range, _trackmin, _trackmax, _lockon, _iflockon, guided = o:weapstats()
         if not guided then
            hasunguided = true
            break
         end
      end
   end

   -- effect goes from 3 to 7
   if hasunguided then
      po:set( "launch_damage", DAMAGEBONUS )
   else
      po:clear()
   end
end

-- Update on outfit change or init
init = update_damage
onoutfitchange = update_damage
