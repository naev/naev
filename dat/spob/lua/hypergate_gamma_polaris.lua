local hypergate = require "spob.lua.lib.hypergate"
hypergate.setup{
   basecol = { 0.2, 0.8, 0.2 }, -- Empire
   cost_mod = {
      [100] = 0,
      [90]  = 0.1,
      [70]  = 0.3,
      [50]  = 0.5,
      [30]  = 0.8,
      [10]  = 0.9,
   },
   tex = "hypergate_empire_activated",
}
