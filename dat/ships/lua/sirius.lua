local explib = require "ships.lua.lib.explode"

local amp = outfit.get("Internal Flow Amplifier")

function init( p )
   -- Sirius ships need the Internal Flow Amplifier for now, add if missing
   -- TODO get rid of the hack
   local found = false
   for k,o in ipairs(p:outfitsList("intrinsic")) do
      if o==amp then
         found = true
         break
      end
   end
   if found then return end

   p:outfitAddIntrinsic( amp )
end

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
