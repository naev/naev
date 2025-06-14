local fmt = require "format"
local corsair = require "outfits.lib.sets.corsair"
local multicore = require "outfits.lib.multicore"

-- Core bonuses
local SPEED_MOD = 25

local lib = {}

function lib.init ()
   corsair.init()

   local descextra_old = descextra
   function descextra( p, o, po )
      local d = "#p"..fmt.f(_("Gain {speed_mod}% speed while stealthed."),
         {speed_mod=SPEED_MOD}).."#0"
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
         po:set("speed_mod", SPEED_MOD)
      end
   end
end

return lib
