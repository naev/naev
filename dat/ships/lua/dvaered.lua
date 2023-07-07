local explib = require "ships.lua.lib.explode"

local function exp_params( size )
   return {
      colorbase = {1.2, 0.9, 0.5, 0.5},
      colorsmoke = {0.1, 0.1, 0.1, 0.1},
      smokiness = 0.6,
      smokefade = 1.6,
      rollspeed = 0.6,
      grain = 0.1 + size*0.00125,
   }
end

explode_init, explode_update = explib{
   dtimer_mod = 0.7, -- faster explosions
   boom_size_mod = 1.4,
   exp_size_mod = 1.2,
   boom_params = exp_params,
   exp_params = exp_params,
   dmgtype = "impact",
}
