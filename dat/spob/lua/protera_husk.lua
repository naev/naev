local luaspob = require "spob.lua.lib.spob"

function init( spb )
   mem.spob = spb
end

function barbg ()
   return luaspob.bg_generator{
      colbg    = { 0.5, 0.3, 0.5, 1 },
      colfeat  = { 0.2, 0.2, 0.6, 1 },
      collight = { 1.0, 0.9, 1.0, 1 },
      featrnd  = { 0.2, 0.4, 0.1 },
      featalpha = 0.4,
      featrandonmess = 0.2,
      nlights  = rnd.rnd(6,8),
   }
end

function population ()
   return p_("population","???")
end

function comm ()
   return false, _("The communication channel is only static.")
end

function can_land ()
   return true, _("Landing port seems to be open.")
end
