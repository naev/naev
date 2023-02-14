local explib = require "ships.lua.lib.explode"

local function exp_params( _size )
   return {
      colorbase = {1.0, 0.8, 0.8, 0.7},
      colorsmoke = {0.3, 0.3, 0.3, 0.1},
      smokiness = 0.4,
      --smokefade = 1.6,
      --rollspeed = 0.3,
      --grain = 0.1 + size*0.001,
   }
end

explode_init, explode_update = explib{
   boom_params = exp_params,
   exp_params = exp_params,
}
