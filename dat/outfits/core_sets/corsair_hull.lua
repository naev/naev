local fmt = require "format"
local corsair = require "outfits.lib.sets.corsair"
local multicore = require "outfits.lib.multicore"

-- Core bonuses
local ARMOUR_REGEN = 2

local lib = {}

function lib.init ()
   corsair.init()

   local descextra_old = descextra
   function descextra( p, o, po )
      local d = "#p"..fmt.f(_("Gain {aregen} {unit} armour regeneration while stealthed."),
         {aregen=ARMOUR_REGEN, unit=naev.unit("power")}).."#0"
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
      multicore.set( p, po )
      if stealthed then
         po:set("armour_regen", ARMOUR_REGEN)
      end
   end
end

return lib
