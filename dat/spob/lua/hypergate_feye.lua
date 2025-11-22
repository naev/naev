local hypergate = require "spob.lua.lib.hypergate"
hypergate.setup{
   basecol = { 0.2, 0.8, 0.8 }, -- Soromid
   cost_mod = {
      [100] = 0,
      [70]  = 0.2,
      [50]  = 0.4,
      [30]  = 0.8,
   },
   tex = "hypergate_soromid_activated",
}
