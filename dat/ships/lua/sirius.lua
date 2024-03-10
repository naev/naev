local explosion = require "luaspfx.explosion"
local chakra = require "luaspfx.chakra_explosion"

local explib = require "ships.lua.lib.explode"

local exp_params = {
   colourbase = {1.0, 0.8, 0.8, 0.7},
   coloursmoke = {0.3, 0.3, 0.3, 0.1},
   smokiness = 0.4,
   --smokefade = 1.6,
   --rollspeed = 0.3,
   --grain = 0.1 + size*0.001,
}

local amp = outfit.get("Internal Flow Amplifier")

function init( p )
   -- Sirius ships need the Internal Flow Amplifier for now, add if missing
   -- TODO get rid of before 0.12.0 release, needed to upgrade older ships atm (new ones just add the intrinsic outfit)
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

explode_init, explode_update = explib{
   boom_func = function( p )
      mem.timer = 0.09 * (mem.dtimer - mem.timer) / mem.dtimer

      local pos = p:pos() + vec2.newP( mem.r*rnd.rnd(), rnd.angle() )

      if rnd.rnd() < 0.6 then
         local r = mem.r * (0.2 + 0.3 * rnd.rnd())
         explosion( pos, p:vel(), r, nil, tmerge( {silent=true}, exp_params ) )
      else
         local r = mem.r * (0.4 + 0.2 * rnd.rnd())
         chakra( pos, p:vel(), r, nil, {silent=true} )
      end
   end,
   exp_func = function( p )
      local a = math.sqrt( p:mass() )
      local r = mem.r * 1.2 + a
      local d = math.max( 0, 2 * (a * (1+math.sqrt(p:fuel()+1)/28)))
      local prms = {
         penetration = 1,
         dmgtype = "ion",
         disable = 0.2,
      }
      spfx.debris( p:mass(), mem.r, p:pos(), p:vel() )
      chakra( p:pos(), p:vel(), r )
      explosion( p:pos(), p:vel(), r*0.8, d, tmerge( prms, exp_params ) )
      p:cargoJet("all")
   end,
}
