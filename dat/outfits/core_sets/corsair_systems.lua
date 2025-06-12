local fmt = require "format"
local corsair = require "outfits.lib.sets.corsair"

-- Core bonuses
local SHIELD_REGEN = 5
local ENERGY_REGEN = 16

local lib = {}

function lib.init( multicore, set )
   corsair.init( set )

   local descextra_old = descextra
   function descextra( p, o, po )
      local d = "#p"..fmt.f(_("Gain an additional {sregen} {unit} shield and {eregen} {unit} energy regeneration while stealthed."),
         {sregen=SHIELD_REGEN, eregen=ENERGY_REGEN, unit=naev.unit("power")}).."#0"
      if descextra_old then
         d=d.."\n"..descextra_old( p, o, po )
      end
      return d
   end

   local onstealth_old = onstealth
   function onstealth( p, po, stealthed )
      if onstealth_old then
         onstealth_old( p, po, stealthed )
      end
      po:clear()
      multicore.set()
      if stealthed then
         po:set("shield_regen", SHIELD_REGEN)
         po:set("energy_regen", ENERGY_REGEN)
      end
   end
end

return lib
