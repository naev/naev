local fmt = require "format"

notactive = true
local RANGE = 3000
local BONUS = 30

function descextra( _p, _o, _po )
   return fmt.f(_("Gives +{bonus}% shield regeneration to all friendly ships within {range} units. Range is affected by detection bonus."),{
      range = RANGE,
      bonus = BONUS,
   })
end

-- Init function run on creation
local t
function init( p, _po )
   t = 0
   p:effectAdd("Shield Aura", math.huge)
end
function update( p, _po, dt )
   t = t + dt
   if t < 1 then
      return
   end

   local mod = p:shipstat("ew_detect",true)
   t = t-1
   for k,a in ipairs(p:getAllies( RANGE*mod, nil, nil, nil, true )) do
      a:effectAdd("Shield Aura")
   end
end
